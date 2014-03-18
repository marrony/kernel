#include <stdint.h>
#include "asm.h"
#include "irq.h"
#include "kprintf.h"

void timer_callback(const struct registers_t* regs) {
    static uint32_t ticks = 0;

    ticks++;
    kprintf("\rclock ticks: %u", ticks);
}

void init_timer(uint32_t frequency) {
    register_irq(IRQ0, &timer_callback);

    uint32_t divisor = 1193182 / frequency;

    outb(0x43, 0x36);
    outb(0x40, divisor & 0xff);
    outb(0x40, (divisor >> 8) & 0xff);
}

extern void init_pic8259();

int kmain() {
    init_pic8259();

    kprintf("THIS IS MY AWESOME KERNEL\n");
    kprintf("AUTHOR: MARRONY N. NERIS\n");
    kprintf("VERSION: 1.0\n\n");

    __asm__ __volatile__ ("int $30");

    init_timer(19);

    while(1)
        hlt();

    return 0;
}

