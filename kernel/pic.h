#ifndef PIC_H
#define PIC_H

#include <stdint.h>
#include "io.h"

void pic_init(void);
void pic_send_eoi(unsigned char irq);

#endif
