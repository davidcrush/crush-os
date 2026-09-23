#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define PIC_VECTOR_TIMER 0x20
#define PIC_VECTOR_KBD 0x21

void pic_init(void);
void pic_send_eoi(uint8_t irq);

#endif
