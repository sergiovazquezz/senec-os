#include "vga.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t row;
    size_t column;
    uint8_t color;
    uint16_t* buffer;
} terminal_t;

static terminal_t term = { .buffer = (uint16_t*)VGA_MEMORY_VA };

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8;
}

void vga_init()
{
    term.row = 0;
    term.column = 0;
    term.color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            term.buffer[index] = vga_entry(' ', term.color);
        }
    }
}

static void vga_put_entry_at(char c, uint8_t color, size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    term.buffer[index] = vga_entry(c, color);
}

void vga_putc(char c)
{
    if (c == '\n') {
        term.column = 0;

        if (++term.row == VGA_HEIGHT)
            term.row = 0;
        return;
    }

    vga_put_entry_at(c, term.color, term.column, term.row);

    if (++term.column == VGA_WIDTH) {
        term.column = 0;

        if (++term.row == VGA_HEIGHT)
            term.row = 0;
    }
}
