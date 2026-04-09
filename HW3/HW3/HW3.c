#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5
#define MCP23008_ADDR 0x20
#define IODIR 0x00
#define GPIO_REG 0x09
#define OLAT 0x0A
#define HB 25

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

int main() {
    stdio_init_all();
    sleep_ms(2000);

    gpio_init(HB);
    gpio_set_dir(HB, GPIO_OUT);

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    setPin(MCP23008_ADDR, IODIR, 0x7F);

    while (true) {
        bool heartbeat = gpio_get(HB);
        gpio_put(HB, !heartbeat);

        uint8_t gpio_val = readPin(MCP23008_ADDR, GPIO_REG);
        if (!(gpio_val & 0x01)) {
            setPin(MCP23008_ADDR, OLAT, 0x80);
        } else {
            setPin(MCP23008_ADDR, OLAT, 0x00);
        }

        sleep_ms(500);
    }
}