#include "serial.h"

#include <stdbool.h>
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value)
{
    asm volatile("out dx, al" : : "a"(value), "d"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    asm volatile("in al, dx" : "=a"(value) : "d"(port));
    return value;
}

static inline bool is_transmit_empty()
{
    return (inb(COM1 + 5) & 0x20) != 0;
}

void serial_init()
{
    outb(COM1 + 1, 0x00); // Disable interrupts
    outb(COM1 + 3, 0x80); // Enable DLAB
    outb(COM1 + 0, 0x03); // 38400 baud (divisor lo)
    outb(COM1 + 1, 0x00); // (divisor hi)
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7); // Enable FIFO
    outb(COM1 + 4, 0x0B); // RTS/DSR set
}

void serial_putc(char c)
{
    while (!is_transmit_empty())
        ;

    outb(COM1, (uint8_t)(c));
}
