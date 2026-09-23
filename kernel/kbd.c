#include "kbd.h"

#include "io.h"
#include "pic.h"
#include "printk.h"

#include <stdint.h>

void kbd_init(void) {}

void kbd_on_keypress(void)
{
    uint8_t scancode = inb(0x60);

    if ((scancode & 0x80) == 0) {
        printk("Scancode: %x\n", scancode);
    }

    pic_send_eoi(1);
}
