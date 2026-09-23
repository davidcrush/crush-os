#include "serial.h"
#include "printk.h"
#include "idt.h"
#include "pic.h"
#include "timer.h"
#include "kbd.h"

static void init(void) {
    serial_init();
    idt_init();
    pic_init();
    timer_init();
    kbd_init();
}

static inline void sti(void) {
    __asm__ volatile ("sti");
}

void kmain(void) {
    init();
    sti();
    printk("Hello, World! My name is David.\nWhoop Whoop!\nWelcome to my kernel!");
    printk("\nThe number 1234567890 in decimal is %d\n", 1234567890);
    printk("\nThe number 1234567890 in hexadecimal is %x\n", 1234567890);
    printk("\nThe number -1234567890 in decimal is %d\n", -1234567890);
    printk("\nThe number -1234567890 in hexadecimal is %x\n", -1234567890);
    //force_exception();
    printk("\nThe string \"Hello, World!\" is %s\n", "Hello, World!");
    for (;;) { __asm__ __volatile__("hlt"); }
}
