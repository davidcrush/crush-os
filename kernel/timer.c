#include "timer.h"

#include "io.h"
#include "pic.h"
#include "printk.h"

#include <stdint.h>

#define HZ 100 /* 100 Hz = 10 ms per tick */

#define PIT_CH0 0x40
#define PIT_CMD 0x43
#define PIT_BASE_HZ 1193182

static uint32_t ticks = 0;

void timer_init(void)
{
    unsigned int divisor = PIT_BASE_HZ / HZ;

    outb(PIT_CMD, 0x36); /* ch0, lobyte/hibyte, mode 3 */
    outb(PIT_CH0, divisor & 0xFF);
    outb(PIT_CH0, (divisor >> 8) & 0xFF);
}

void timer_on_tick(void)
{
    ticks++;
    if (ticks % HZ == 0) {
        printk("Tick: %d\n", ticks);
    }
    pic_send_eoi(0);
}
