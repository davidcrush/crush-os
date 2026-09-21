#include "console.h"
#include "serial.h"

void kmain(void) {
    serial_init();
    console_write("Hello, World! My name is David.\nWhoop Whoop!\nWelcome to my kernel!");
    for (;;) { __asm__ __volatile__("hlt"); }
}
