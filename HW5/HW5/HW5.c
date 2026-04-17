#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/adc.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5
#define HB 25
#define MPU_ADDR 0x68
// config registers
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C
// sensor data registers:
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H   0x41
#define TEMP_OUT_L   0x42
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48
#define WHO_AM_I     0x75



void drawLetter(unsigned char x, unsigned char y, char c);
void drawMessage(unsigned char x, unsigned char y, char * m);



void mpu6050_read(uint8_t *buf){
    uint8_t r = ACCEL_XOUT_H;
    i2c_write_blocking(I2C_PORT, MPU_ADDR, &r, 1, true);
    i2c_read_blocking(I2C_PORT, MPU_ADDR, buf, 14, false);
}

void setPin(unsigned char address, unsigned char reg, unsigned char value) {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    i2c_write_blocking(I2C_PORT, address, buf, 2, false);
}

unsigned char readPin(unsigned char address, unsigned char reg) {
    uint8_t val;
    uint8_t r = reg;
    i2c_write_blocking(I2C_PORT, address, &r, 1, true);
    i2c_read_blocking(I2C_PORT, address, &val, 1, false);
    return val;
}

void drawLine(int x0, int y0, int x1, int y1)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        ssd1306_drawPixel(x0, y0, 1);
        if (x0 == x1 && y0 == y1) break;
        int err2 = err * 2;
        if (err2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (err2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void drawMessage(unsigned char x, unsigned char y, char * m){
    while(*m != '\0'){
        drawLetter(x, y, *m);
        m++;
        x+=6;
    }
}


void drawLetter(unsigned char x, unsigned char y, char c){
   
    for (int i = 0; i < 5; i++){
        unsigned char col = ASCII[c-32][i];
        for (int j = 0; j < 8; j++)
        {
            ssd1306_drawPixel(x+i, y+j, (col >> j) & 1);
        }
    }
}

int main()
{
    stdio_init_all();
    gpio_init(HB);
    gpio_set_dir(HB, GPIO_OUT);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    ssd1306_setup();
    
    adc_init();
    adc_gpio_init(26); // GPIO26 is ADC0

    readPin(MPU_ADDR, WHO_AM_I);
    setPin(MPU_ADDR, PWR_MGMT_1, 0x00);
    setPin(MPU_ADDR, ACCEL_CONFIG, 0x00);  
    setPin(MPU_ADDR, GYRO_CONFIG, 0x18);

    unsigned int t0 = to_us_since_boot(get_absolute_time());
    while (true) {
    
        // // if(readPin(MPU_ADDR, WHO_AM_I) == 0x68){
        // //     drawMessage(0, 0, "MPU is alive!");
        // // } else {
        // //     while(1){
        // //         gpio_put(HB, 1);
        // //     }
        // // }
        uint8_t raw[14];
        mpu6050_read(raw);

        int16_t accel_x = (int16_t)((raw[0] << 8) | raw[1]);
        int16_t accel_y = (int16_t)((raw[2] << 8) | raw[3]);
        int endX = 64 + (int)(accel_x * 0.000061 * 25);
        int endY = 16 + (int)(accel_y * 0.000061 * 12); 

        if (endX < 0) endX = 0; 
        if (endX > 127) endX = 127;
        if (endY < 0) endY = 0; 
        if (endY > 31) endY = 31;

        ssd1306_clear();
        drawLine(64, 16, endX, endY);
        
        bool heartbeat = gpio_get(HB);
        gpio_put(HB, !heartbeat);
        ssd1306_drawPixel(0, 0, heartbeat);

        ssd1306_update();
        sleep_ms(10);
    }
}
