#include "mpu6050.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

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

void mpu6050_init(void) {
    i2c_init(I2C_PORT, 400*1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    setPin(MPU_ADDR, PWR_MGMT_1,   0x00);
    setPin(MPU_ADDR, ACCEL_CONFIG, 0x00);
    setPin(MPU_ADDR, GYRO_CONFIG,  0x18);
}

void mpu6050_read(uint8_t *buf) {
    uint8_t r = ACCEL_XOUT_H;
    i2c_write_blocking(I2C_PORT, MPU_ADDR, &r, 1, true);
    i2c_read_blocking(I2C_PORT, MPU_ADDR, buf, 14, false);
}