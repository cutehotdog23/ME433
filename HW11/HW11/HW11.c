#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

int main() {
    stdio_init_all();
    sleep_ms(2000);  // wait for USB to enumerate

    printf("Pico booted!\n");

    // Initialize UART0 for STM32 communication
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    while (1) {
        // If STM32 sent something over UART, forward to computer (USB)
        if (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);
            putchar(c);         // forward to computer
            stdio_flush();
        }

        // If computer sent something over USB, forward to STM32 and echo back
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            putchar(c);                     // echo back to computer so you see what you typed
            stdio_flush();
            uart_putc(UART_ID, (char)c);    // forward to STM32
        }
    }

    return 0;
}