#ifndef MPU6050_H
#define MPU6050_H

#include "hardware/i2c.h"

// I2C config
#define I2C_PORT i2c0
#define I2C_SDA  4
#define I2C_SCL  5
#define MPU_ADDR 0x68

// Registers
#define ACCEL_XOUT_H 0x3B
#define PWR_MGMT_1   0x6B
#define ACCEL_CONFIG 0x1C
#define GYRO_CONFIG  0x1B

// Function declarations
void mpu6050_init(void);
void mpu6050_read(uint8_t *buf);
void setPin(unsigned char address, unsigned char reg, unsigned char value);
unsigned char readPin(unsigned char address, unsigned char reg);

#endif