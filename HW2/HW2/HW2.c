#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#define PIN 16
#define MIN_PULSE_WIDTH 500
#define MAX_PULSE_WIDTH 2500
#define MIN_ANGLE 0
#define MAX_ANGLE 180
uint16_t wrap;
float div;


uint16_t angle_to_duty(int angle) {
    // Map the angle to the pulse width
    float pulse_width = MIN_PULSE_WIDTH + (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) * ((float)(angle - MIN_ANGLE) / (MAX_ANGLE - MIN_ANGLE));
    // Map the pulse width to the duty cycle
    return (uint16_t)(pulse_width);
}
int main()
{
    stdio_init_all();
    gpio_set_function(PIN, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(PIN); // Get PWM slice number
    // the clock frequency is 150MHz divided by a float from 1 to 255
    float div = 150.0; // must be between 1-255
    pwm_set_clkdiv(slice_num, div); // sets the clock speed
    wrap = 19999; // when to rollover, must be less than 65535
    // set the PWM frequency and resolution
    // this sets the PWM frequency to 150MHz / div / wrap
    pwm_set_wrap(slice_num, wrap); 
    pwm_set_enabled(slice_num, true); // turn on the PWM

    while (true) {
        for (int i = 0; i <180; i++) {
            pwm_set_gpio_level(PIN, angle_to_duty(i));
            sleep_ms(10); // wait for 1 ms
        }
         for (int i = 180; i > 0; i--) {
            pwm_set_gpio_level(PIN, angle_to_duty(i)); // decrease the duty cycle
            sleep_ms(10); // wait for 1 ms
        }

        
    }
}
