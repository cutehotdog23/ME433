#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>

#define SPI_PORT spi0
#define PIN_CS 14

void writeDAC(int channel, float v);


static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}

void writeDAC(int channel, float v)
{
    uint8_t data[2];

    uint16_t myVoltage = (uint16_t)(v / 3.3f * 1023);

    data[0] = 0b00110000;
    data[0] |= (channel & 0b1) << 7;
    data[0] |= (myVoltage >> 6) & 0b00001111;
    data[1]  = (myVoltage << 2) & 0xFF;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}

int main()
{
    spi_init(SPI_PORT, 20000 * 1000); // the baud, or bits per second
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    stdio_init_all();

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS);
    
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    float t = 0.0;


    while (true) 
    {
        // call writeDAC
        float voltage = (sinf(2*M_PI*2.0 * 1.0*t) +1.0)/2.0*3.3;
        writeDAC(0, (uint16_t)(voltage / 3.3 * 1023));

        float tri_phase = fmodf(t, 1.0);
        float tri_v;
        if (tri_phase < 0.5) 
        {
        tri_v = tri_phase * 2.0 * 3.3;
        } else 
        {
        tri_v = (1.0 - tri_phase) * 2.0 * 3.3;
        }
        writeDAC(1, (uint16_t)(tri_v / 3.3 * 1023));
        t = t+0.01;
        sleep_ms(10);        
    }
}


