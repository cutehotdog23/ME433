#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
 
#define SCK_PIN 2   
#define DT_PIN  3   
 

#define IIR_A 0.85f
#define IIR_B (1.0f - IIR_A)
 
#define MAX_SAMPLES 1000
 
void hx711_init(void) {
    gpio_init(SCK_PIN);
    gpio_set_dir(SCK_PIN, GPIO_OUT);
    gpio_put(SCK_PIN, 0);
 
    gpio_init(DT_PIN);
    gpio_set_dir(DT_PIN, GPIO_IN);
}
 
// Read 24-bit value from HX711, returns signed int
// After 24 data clocks, 1 extra clock sets gain=128 for next read
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
 
int main(void) {
    stdio_init_all();
    hx711_init();
 
    int32_t raw_data[MAX_SAMPLES];
    float   filt_data[MAX_SAMPLES];
    uint32_t time_ms[MAX_SAMPLES];
 
    while (1) {
        int n = 0;
        scanf("%d", &n);
        if (n <= 0 || n > MAX_SAMPLES) {
            printf("ERROR: n must be 1-%d\n", MAX_SAMPLES);
            continue;
        }
 
        // Collect n samples
        float iir_prev = (float)hx711_read(); // prime the filter
        for (int i = 0; i < n; i++) {
            uint32_t t0 = to_ms_since_boot(get_absolute_time());
            int32_t raw = hx711_read();
            float filt = IIR_A * iir_prev + IIR_B * (float)raw;
            iir_prev = filt;
 
            raw_data[i]  = raw;
            filt_data[i] = filt;
            time_ms[i]   = t0;
        }
 
        // Send all data back: time_ms, raw, filtered
        for (int i = 0; i < n; i++) {
            printf("%lu,%ld,%.2f\n", (unsigned long)time_ms[i], (long)raw_data[i], filt_data[i]);
        }
    }
 
    return 0;
}
 