#include "pic.h"

#include "io.h"

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

#define PIC2_OFFSET 0x28 /* IRQ8..15 -> vectors 40..47 */

static inline void io_wait(void)
{
    outb(0x80, 0); /* tiny delay between PIC writes */
}

void pic_init(void)
{
    outb(PIC1_CMD, 0x11);
    io_wait(); /* ICW1: init + expect ICW4 */
    outb(PIC2_CMD, 0x11);
    io_wait();

    outb(PIC1_DATA, PIC_VECTOR_TIMER);
    io_wait(); /* ICW2: master vector base */
    outb(PIC2_DATA, PIC2_OFFSET);
    io_wait();

    outb(PIC1_DATA, 0x04);
    io_wait(); /* ICW3: slave on IRQ2 */
    outb(PIC2_DATA, 0x02);
    io_wait(); /* ICW3: slave identity */

    outb(PIC1_DATA, 0x01);
    io_wait(); /* ICW4: 8086 mode */
    outb(PIC2_DATA, 0x01);
    io_wait();

    /* Unmask IRQ0 (timer) and IRQ1 (keyboard). 0 = unmasked. */
    outb(PIC1_DATA, 0xFC);
    outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8) {
        outb(PIC2_CMD, PIC_EOI);
    }
    outb(PIC1_CMD, PIC_EOI);
}
