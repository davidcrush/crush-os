#include "serial.h"
#include "printk.h"

void kmain(void) {
    serial_init();
    printk("Hello, World! My name is David.\nWhoop Whoop!\nWelcome to my kernel!");
    printk("\nThe number 1234567890 in decimal is %d\n", 1234567890);
    printk("\nThe number 1234567890 in hexadecimal is %x\n", 1234567890);
    printk("\nThe number -1234567890 in decimal is %d\n", -1234567890);
    printk("\nThe number -1234567890 in hexadecimal is %x\n", -1234567890);
    printk("\nThe string \"Hello, World!\" is %s\n", "Hello, World!");
    for (;;) { __asm__ __volatile__("hlt"); }
}
