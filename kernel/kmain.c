#include "serial.h"
#include "printk.h"
#include "idt.h"

static void force_exception(void) {
    volatile int zero = 0;
    volatile int x = 1 / zero;
    (void)x;
}

void kmain(void) {
    serial_init();
    idt_init();
    printk("Hello, World! My name is David.\nWhoop Whoop!\nWelcome to my kernel!");
    printk("\nThe number 1234567890 in decimal is %d\n", 1234567890);
    printk("\nThe number 1234567890 in hexadecimal is %x\n", 1234567890);
    printk("\nThe number -1234567890 in decimal is %d\n", -1234567890);
    printk("\nThe number -1234567890 in hexadecimal is %x\n", -1234567890);
    //force_exception();
    printk("\nThe string \"Hello, World!\" is %s\n", "Hello, World!");
    for (;;) { __asm__ __volatile__("hlt"); }
}
