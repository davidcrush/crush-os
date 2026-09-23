#include "printk.h"

#include "console.h"

#include <stdint.h>

static void kputc(char c)
{
    console_putc(c);
}

static void kputs(const char *s)
{
    while (*s) {
        kputc(*s);
        s++;
    }
}

static void print_u32_dec(uint32_t v)
{
    char buf[10]; // max for uint32_t: 4294967295 (10 digits)
    int i = 0;

    if (v == 0) {
        kputc('0');
        return;
    }

    while (v > 0) {
        buf[i++] = (char)('0' + (v % 10));
        v /= 10;
    }

    while (i--) {
        kputc(buf[i]);
    }
}

static void print_u32_hex(uint32_t v)
{
    char buf[8]; // max for uint32_t hex: ffffffff (8 digits)
    int i = 0;
    static const char hex[] = "0123456789abcdef";

    if (v == 0) {
        kputc('0');
        return;
    }

    while (v > 0) {
        buf[i++] = hex[v & 0xF];
        v >>= 4;
    }

    while (i--) {
        kputc(buf[i]);
    }
}

static void print_i32_dec(int32_t v)
{
    if (v < 0) {
        kputc('-');

        // avoid overflow on INT32_MIN:
        // -(INT32_MIN) overflows in signed arithmetic, so do unsigned trick
        uint32_t mag = (uint32_t)(-(v + 1)) + 1U;
        print_u32_dec(mag);
        return;
    }

    print_u32_dec((uint32_t)v);
}

static void vprintk(const char *fmt, va_list ap)
{
    if (!fmt)
        return;

    while (*fmt) {
        if (*fmt != '%') {
            kputc(*fmt++);
            continue;
        }

        fmt++; // skip '%'

        // Handle trailing '%' at end of format
        if (*fmt == '\0') {
            kputc('%');
            break;
        }

        switch (*fmt) {
        case '%':
            kputc('%');
            break;

        case 's': {
            const char *s = va_arg(ap, const char *);
            kputs(s);
            break;
        }

        case 'c': {
            int ch = va_arg(ap, int); // default arg promotion
            kputc((char)ch);
            break;
        }

        case 'd': {
            int32_t v = va_arg(ap, int32_t);
            print_i32_dec(v);
            break;
        }

        case 'u': {
            uint32_t v = va_arg(ap, uint32_t);
            print_u32_dec(v);
            break;
        }

        case 'x': {
            uint32_t v = va_arg(ap, uint32_t);
            print_u32_hex(v);
            break;
        }

        default:
            // Unknown specifier: print it literally as "%?"
            kputc('%');
            kputc(*fmt);
            break;
        }

        fmt++;
    }
}

void printk(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vprintk(format, ap);
    va_end(ap);
}

void panic(const char *msg)
{
    printk("PANIC: %s\n", msg);
    for (;;) {
        __asm__ __volatile__("cli; hlt");
    }
}
