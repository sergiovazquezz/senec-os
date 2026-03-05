#include "printk.h"
#include "serial.h"
#include "vga.h"

#include <stdarg.h>

void printk_init(void)
{
    serial_init();
    vga_init();
}

static void kputc(char c)
{
    if (c == '\n')
        serial_putc('\r');

    serial_putc(c);
    vga_putc(c);
}

static void kputs(const char* s)
{
    while (*s)
        kputc(*s++);
}

static void kputu(uint64_t val, int base)
{
    static const char digits[] = "0123456789abcdef";
    char buf[64];
    int i = 0;

    if (val == 0) {
        kputc('0');
        return;
    }

    while (val) {
        buf[i++] = digits[val % base];
        val /= base;
    }

    while (i--)
        kputc(buf[i]);
}

void printk(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            kputc(*fmt);
            continue;
        }

        switch (*++fmt) {
        case 'c':
            kputc((char)va_arg(ap, int));
            break;
        case 's':
            kputs(va_arg(ap, const char*));
            break;
        case 'd': {
            int64_t v = va_arg(ap, int64_t);
            if (v < 0) {
                kputc('-');
                v = -v;
            }
            kputu((uint64_t)v, 10);
        } break;
        case 'u':
            kputu(va_arg(ap, uint64_t), 10);
            break;
        case 'x':
            kputu(va_arg(ap, uint64_t), 16);
            break;
        case 'p':
            kputs("0x");
            kputu((uint64_t)va_arg(ap, void*), 16);
            break;
        case '%':
            kputc('%');
            break;
        }
    }

    va_end(ap);
}
