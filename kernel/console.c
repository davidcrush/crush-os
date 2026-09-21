#include <stdint.h>
#include <stddef.h>
#include "console.h"
#include "serial.h"

static uint8_t cursor_row = 0;
static uint8_t cursor_col = 0;
static volatile unsigned short *const vga =
    (volatile unsigned short *)0xB8000;

static void console_scroll() {
    // If the cursor is not at the last row, just increment the row
    if (cursor_row < 24) {
        cursor_row++;
        return;
    }

    // Shift the screen up
    for (uint8_t i = 1; i < 25; i++) {
        for (int x = 0; x < 80; x++) {
            vga[(i - 1) * 80 + x] = vga[i * 80 + x];
        }
    }

    // Set the cursor to the last row and column
    cursor_row = 24;
    cursor_col = 0;
    // Clear the last line
    for (int x = 0; x < 80; x++) {
        vga[24 * 80 + x] = 0x0F20; /* space, same attribute */
    }
}

void console_putc(char c) {
    serial_putc(c);

    switch (c) {
        case '\n':
            cursor_col = 0;
            console_scroll();
            break;
        default:
            //console_putc(c);
            vga[cursor_row * 80 + cursor_col] = 0x0F00 | (unsigned char)c;
            cursor_col++;
            if (cursor_col >= 80) {
                cursor_col = 0;
                console_scroll();
            }
            break;
    }
}

void console_write(const char *s) {
    if (!s) return;
    while (*s) {
        console_putc(*s++);
    }
}
