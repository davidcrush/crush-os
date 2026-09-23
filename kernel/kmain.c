#include "idt.h"
#include "kbd.h"
#include "pic.h"
#include "printk.h"
#include "serial.h"
#include "timer.h"

static void init(void)
{
    serial_init();
    idt_init();
    pic_init();
    timer_init();
    kbd_init();
}

static inline void sti(void)
{
    __asm__ volatile("sti");
}

void kmain(void)
{
    init();
    sti();
    printk("Crush OS\n");
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}
