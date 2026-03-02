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

static inline bool is_transmit_empty() { return (inb(COM1 + 5) & 0x20) != 0; }

void serial_init()
{
    outb(COM1 + 1, 0x00); // Disable interrupts
    outb(COM1 + 3, 0x80); // Enable DLAB
    outb(COM1 + 0, 0x03); // 38400 baud (divisor lo)
    outb(COM1 + 1, 0x00); // (divisor hi)
    outb(COM1 + 3, 0x03); // 8N1
    outb(COM1 + 2, 0xC7); // Enable FIFO
    outb(COM1 + 4, 0x0B); // RTS/DSR set
}

void serial_putc(char c)
{
    while (!is_transmit_empty()) {
    }
    outb(COM1, (uint8_t)(c));
}

void serial_puts(const char* str)
{
    while (*str) {
        if (*str == '\n')
            serial_putc('\r');

        serial_putc(*str++);
    }
}

void serial_puth(uint64_t value)
{
    static const char hex[] = "0123456789ABCDEF";
    serial_putc('0');
    serial_putc('x');

    int started = 0;
    for (int i = 60; i >= 0; i -= 4) {
        uint8_t nibble = (value >> i) & 0xF;
        if (nibble || started || i == 0) {
            serial_putc(hex[nibble]);
            started = 1;
        }
    }
}

void serial_putd(uint64_t value)
{
    if (value == 0) {
        serial_putc('0');
        return;
    }

    char buf[20];
    int pos = 0;
    while (value > 0) {
        buf[pos++] = '0' + (char)(value % 10);
        value /= 10;
    }
    for (int i = pos - 1; i >= 0; i--)
        serial_putc(buf[i]);
}
