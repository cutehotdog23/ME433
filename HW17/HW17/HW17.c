#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

// ---------------- HX711 (force) ----------------
#define SCK_PIN 2
#define DT_PIN  3

#define IIR_A 0.85f
#define IIR_B (1.0f - IIR_A)

// ---------------- AS5600 (encoder) ----------------
#define AS5600_ADDR        0x36
#define AS5600_REG_ANGLE_H 0x0E   // raw angle high byte (0x0E:H, 0x0F:L)
#define AS5600_I2C         i2c0
#define AS5600_SDA_PIN     16
#define AS5600_SCL_PIN     17

// ================= HX711 functions =================
void hx711_init(void) {
    gpio_init(SCK_PIN);
    gpio_set_dir(SCK_PIN, GPIO_OUT);
    gpio_put(SCK_PIN, 0);

    gpio_init(DT_PIN);
    gpio_set_dir(DT_PIN, GPIO_IN);
}

// Read 24-bit value from HX711, returns signed int.
// After 24 data clocks, 1 extra clock sets gain=128 for next read.
int32_t hx711_read(void) {
    // Wait until DT goes low (data ready)
    while (gpio_get(DT_PIN) == 1) {
        tight_loop_contents();
    }

    uint32_t raw = 0;

    // Clock out 24 bits
    for (int i = 0; i < 24; i++) {
        gpio_put(SCK_PIN, 1);
        sleep_us(1);
        raw = (raw << 1) | gpio_get(DT_PIN);
        gpio_put(SCK_PIN, 0);
        sleep_us(1);
    }

    // 25th clock pulse - sets gain=128 for next conversion
    gpio_put(SCK_PIN, 1);
    sleep_us(1);
    gpio_put(SCK_PIN, 0);
    sleep_us(1);

    if (raw & 0x800000) {
        raw |= 0xFF000000;
    }

    return (int32_t)raw;
}

// ================= AS5600 functions =================
void as5600_init(void) {
    i2c_init(AS5600_I2C, 400000);  // 400kHz fast mode
    gpio_set_function(AS5600_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(AS5600_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(AS5600_SDA_PIN);
    gpio_pull_up(AS5600_SCL_PIN);
}

// Returns 12-bit angle (0-4095 for 0-360 degrees)
uint16_t as5600_read_angle(void) {
    uint8_t reg = AS5600_REG_ANGLE_H;
    uint8_t buf[2];

    i2c_write_blocking(AS5600_I2C, AS5600_ADDR, &reg, 1, true);
    i2c_read_blocking(AS5600_I2C, AS5600_ADDR, buf, 2, false);

    return ((buf[0] & 0x0F) << 8) | buf[1];
}

// ================= Main =================
int main(void) {
    stdio_init_all();
    hx711_init();
    as5600_init();

    // Prime the force filter
    float iir_prev = (float)hx711_read();

    while (1) {
        uint32_t t  = to_ms_since_boot(get_absolute_time());
        int32_t raw = hx711_read();
        float filt  = IIR_A * iir_prev + IIR_B * (float)raw;
        iir_prev    = filt;

        uint16_t angle = as5600_read_angle();

        // Stream: time, angle, raw force, filtered force
        printf("%lu,%u,%ld,%.2f\n",
               (unsigned long)t, angle, (long)raw, filt);
    }

    return 0;
}