#include "kbd.h"
#include "io.h"
#include "printk.h"
#include "pic.h"
#include <stdint.h>

void kbd_init(void) {
    // Nothing to do here yet
}

void kbd_on_keypress(void) {
    // TODO: Implement keyboard on keypress
    uint8_t scancode = inb(0x60);

    if (scancode & 0x80) {
        // key released (break code)
        pic_send_eoi(1);
        return;
    }

    printk("Scancode: %x\n", scancode);
    pic_send_eoi(1);
}
