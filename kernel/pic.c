#include "pic.h"

#define PIC1_CMD   0x20
#define PIC1_DATA  0x21
#define PIC2_CMD   0xA0
#define PIC2_DATA  0xA1
#define PIC_EOI    0x20

#define PIC1_OFFSET 0x20   // IRQ0..7  -> vectors 32..39
#define PIC2_OFFSET 0x28   // IRQ8..15 -> vectors 40..47

static inline void io_wait(void) { outb(0x80, 0); } // tiny delay (legacy-safe)

static void pic_remap(void) {
    unsigned char m1 = inb(PIC1_DATA);  // save masks
    unsigned char m2 = inb(PIC2_DATA);

    outb(PIC1_CMD, 0x11); io_wait();    // ICW1: init + expect ICW4
    outb(PIC2_CMD, 0x11); io_wait();

    outb(PIC1_DATA, PIC1_OFFSET); io_wait(); // ICW2: vector offset
    outb(PIC2_DATA, PIC2_OFFSET); io_wait();

    outb(PIC1_DATA, 0x04); io_wait();   // ICW3: slave on IRQ2
    outb(PIC2_DATA, 0x02); io_wait();   // ICW3: slave identity

    outb(PIC1_DATA, 0x01); io_wait();   // ICW4: 8086 mode
    outb(PIC2_DATA, 0x01); io_wait();

    // Checkpoint 2: unmask only IRQ0 (timer), mask everything else
    outb(PIC1_DATA, 0xFC);  // 11111100b
    outb(PIC2_DATA, 0xFF);  // all masked on slave

    // (optional: restore m1/m2 later when enabling more IRQs)
    (void)m1; (void)m2;
}

void pic_init(void) {
   pic_remap();
}

void pic_send_eoi(unsigned char irq) {
    if (irq >= 8) outb(PIC2_CMD, PIC_EOI); // slave first if needed
    outb(PIC1_CMD, PIC_EOI);               // then master
}
