#include <stdio.h>
#include "pico/stdlib.h"
#include <math.h>
#include "hardware/spi.h"

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS_DAC 17
#define PIN_CS_RAM 20
#define PIN_SCK 18
#define PIN_MOSI 19

union FloatBytes {
            float f;
            uint8_t bytes[4];
        };


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

void update_dac(uint8_t channel, float voltage);
void update_dac_from_ram(int);

void spi_ram_init();
void spi_ram_write(uint16_t, uint8_t *, int);
void spi_ram_read(uint16_t, uint8_t *, int);

void ram_write_sine();


int main()
{
    stdio_init_all();
    sleep_ms(5000);

    spi_init(SPI_PORT, 1000 * 1000 * 2);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);
    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_put(PIN_CS_RAM, 1);

    spi_ram_init();
    ram_write_sine();

    uint8_t dbg[2];
    for (int i = 0; i < 5; i++) 
    {
    spi_ram_read(i * 2, dbg, 2);
    uint16_t val = (dbg[0] << 8) | dbg[1];
    printf("sample %d: 0x%04X\n", i, val);
}

    int i = 0;

    while (true) {
        for (i = 0; i< 1000; i=i+2) {
            update_dac_from_ram(i);
            sleep_ms(1);

        // union FloatBytes fb_write;
        // fb_write.f = 1.23f;
        // spi_ram_write(0, fb_write.bytes, 4);
        // union FloatBytes fb_read;
        // spi_ram_read(0, fb_read.bytes, 4);
        // printf("Wrote %f, read back %f\n", fb_write.f, fb_read.f);
    }
}
}


void spi_ram_init(){
    uint8_t data[2];
    int len = 2;
    data[0] = 0b00000001;
    data[1] = 0b01000000;
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, data, len);
    cs_deselect(PIN_CS_RAM);
}

void update_dac_from_ram(int i){
    uint8_t data[2];
    spi_ram_read(i, data, 2);

    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS_DAC);
}

void ram_write_sine(){
    int i = 0;
    uint8_t data[2];
    uint16_t data_short = 0;
    uint8_t channel = 0b0;
    float voltage = 0;
    uint16_t addr = 0;

    for (int i = 0; i< 101000; i++) 
    {
        data_short = (channel&0b1)<<15;
        data_short = data_short | (0b111<<12);

        voltage = (sin(2*M_PI*i/1000.0)+1)*512;

        // uint16_t v = (uint16_t)voltage & 0x3FF;
        uint16_t v = voltage;
        data_short = data_short | (0b11111111111 & v);


        data[0] = data_short >>8;
        data[1] = data_short & 0xFF;

        spi_ram_write(addr, data, 2);
        addr = addr+2;

    }

    
}

void spi_ram_write(uint16_t addr, uint8_t *data, int len){
    uint8_t packet[5];
    packet[0] = 0b00000010;
    packet[1] = addr>>8;
    packet[2] = addr & 0xFF;
    packet[3] = data[0];
    packet[4] = data[1];

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, packet, 5);
    cs_deselect(PIN_CS_RAM);
}

void spi_ram_read(uint16_t addr, uint8_t *data, int len){
    uint8_t packet[5];
    packet[0] = 0b00000011;
    packet[1] = addr>>8;
    packet[2] = addr & 0xFF;
    packet[3] = 0;
    packet[4] = 0;
    uint8_t dst[5];
    cs_select(PIN_CS_RAM);
    spi_write_read_blocking(SPI_PORT, packet, dst, 5);
    cs_deselect(PIN_CS_RAM);
    data[0] = dst[3];
    data[1] = dst[4];
}
