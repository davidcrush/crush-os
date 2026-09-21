#include "console.h"

void kmain(void) {
    console_write("Hello, World! My name is David.\nWhoop Whoop!\nWelcome to my kernel!");
    for (;;) { __asm__ __volatile__("hlt"); }
}
