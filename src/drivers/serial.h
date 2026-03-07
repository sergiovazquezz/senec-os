#ifndef DRIVERS_SERIAL_H
#define DRIVERS_SERIAL_H

#include <stddef.h>
#include <stdint.h>

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t value)
{
    asm volatile("out dx, al" : : "a"(value), "d"(port));
}

void serial_init();
void serial_putc(char c);
void serial_puts(const char* str);
void serial_puth(uint64_t value);
void serial_putd(uint64_t value);

#endif // DRIVERS_SERIAL_H
