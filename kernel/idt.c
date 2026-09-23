#include <stdint.h>
#include "idt.h"
#include "printk.h"

#define IDT_COUNT 256
#define KERNEL_CS 0x08
#define IDT_TYPE_INTERRUPT_GATE 0x8E  // present=1, DPL=0, 32-bit interrupt gate

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed));

static struct idt_entry idt[IDT_COUNT];

struct idtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// from isr_stubs.S
extern void* isr_stub_table[IDT_COUNT];

static inline void lidt(const struct idtr* p) {
    __asm__ volatile ("lidt (%0)" : : "r"(p));
}

static void idt_set_gate(uint8_t vec, uint32_t handler_addr) {
    idt[vec].offset_low  = (uint16_t)(handler_addr & 0xFFFF);
    idt[vec].selector    = KERNEL_CS;
    idt[vec].zero        = 0;
    idt[vec].type_attr   = IDT_TYPE_INTERRUPT_GATE;
    idt[vec].offset_high = (uint16_t)((handler_addr >> 16) & 0xFFFF);
}

struct isr_frame {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t vector;
    uint32_t error;
    uint32_t eip, cs, eflags;
};

void isr_dispatch_c(struct isr_frame *frame) {
    printk("exception %u\n", frame->vector);
    panic("exception");
}

void idt_init(void) {
    // Fill all entries so no slot is empty (empty can triple-fault)
    for (uint16_t i = 0; i < IDT_COUNT; i++) {
        idt_set_gate((uint8_t)i, (uint32_t)isr_stub_table[i]);
    }

    struct idtr idtr_value;
    idtr_value.limit = (uint16_t)(sizeof(idt) - 1);   // 256*8 - 1
    idtr_value.base  = (uint32_t)&idt[0];

    // keep interrupts disabled (cli) in checkpoint 1; just load the table
    lidt(&idtr_value);
}
