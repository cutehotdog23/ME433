#include <stdio.h>
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


void drawLetter(unsigned char x, unsigned char y, char c);
void drawMessage(unsigned char x, unsigned char y, char * m);

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

    unsigned int t0 = to_us_since_boot(get_absolute_time());
    while (true) {
        ssd1306_clear();
        bool heartbeat = gpio_get(HB);
        gpio_put(HB, !heartbeat);
        ssd1306_drawPixel(127, 1, !heartbeat);
  

        uint16_t voltage = adc_read();
        float voltage_f = voltage * 3.3 / 4095.0;
        char volt_msg[50];
        sprintf(volt_msg, "Voltage: %.2f V", voltage_f);
        drawMessage(0, 0, volt_msg);

        unsigned int t1 = to_us_since_boot(get_absolute_time());
        float fps = 1000000.0f / (t1 - t0);
        t0 = t1;

        char fps_msg[50];
        sprintf(fps_msg, "FPS: %.2f", fps);
        drawMessage(0, 16, fps_msg);


        ssd1306_update();
        sleep_ms(500);
    }
}
