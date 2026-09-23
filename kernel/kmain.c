#include "serial.h"
#include "printk.h"
#include "idt.h"
#include "pic.h"
#include "timer.h"

// static void force_exception(void) {
//     volatile int zero = 0;
//     volatile int x = 1 / zero;
//     (void)x;
// }

static void init(void) {
    serial_init();
    idt_init();
    pic_init();
    timer_init();
}

void kmain(void) {
    init();
    printk("Hello, World! My name is David.\nWhoop Whoop!\nWelcome to my kernel!");
    printk("\nThe number 1234567890 in decimal is %d\n", 1234567890);
    printk("\nThe number 1234567890 in hexadecimal is %x\n", 1234567890);
    printk("\nThe number -1234567890 in decimal is %d\n", -1234567890);
    printk("\nThe number -1234567890 in hexadecimal is %x\n", -1234567890);
    //force_exception();
    printk("\nThe string \"Hello, World!\" is %s\n", "Hello, World!");
    for (;;) { __asm__ __volatile__("hlt"); }
}
