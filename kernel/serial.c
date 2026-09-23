#include "serial.h"

#include "io.h"

#include <stdint.h>

#define COM1 0x3F8

#define REG_DATA 0 /* THR (write) / RBR (read) when DLAB=0 */
#define REG_IER 1  /* Interrupt Enable Register when DLAB=0 */
#define REG_DLL 0  /* Divisor Latch Low when DLAB=1 */
#define REG_DLM 1  /* Divisor Latch High when DLAB=1 */
#define REG_FCR 2  /* FIFO Control Register */
#define REG_LCR 3  /* Line Control Register */
#define REG_MCR 4  /* Modem Control Register */
#define REG_LSR 5  /* Line Status Register */

#define LCR_DLAB 0x80
#define LCR_8N1 0x03 /* 8 data bits, no parity, 1 stop */

#define LSR_THRE 0x20 /* Transmit Holding Register Empty (bit 5) */

#define MCR_DTR 0x01
#define MCR_RTS 0x02
/* OUT2 stays off so the UART does not raise an IRQ. */

void serial_init(void)
{
    outb(COM1 + REG_IER, 0x00);

    /* Divisor 1 is 115200 baud. DLAB must be set to reach DLL/DLM. */
    outb(COM1 + REG_LCR, LCR_DLAB);
    outb(COM1 + REG_DLL, 0x01);
    outb(COM1 + REG_DLM, 0x00);

    outb(COM1 + REG_LCR, LCR_8N1);

    outb(COM1 + REG_FCR, 0x07);

    outb(COM1 + REG_MCR, MCR_DTR | MCR_RTS);

    outb(COM1 + REG_IER, 0x00);
}

void serial_putc(char c)
{
    while (!(inb(COM1 + REG_LSR) & LSR_THRE)) {
    }
    outb(COM1 + REG_DATA, (uint8_t)c);
}
