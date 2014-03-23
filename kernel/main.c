#include <stdint.h>
#include "asm.h"
#include "irq.h"
#include "regs.h"
#include "kprintf.h"

void timer_callback(const struct registers_t* regs) {
    static uint32_t ticks = 0;

    ticks++;
    kprintf("\rclock ticks: %u", ticks);
}

void init_timer(uint32_t frequency) {
    register_interrupt_handler(IRQ0, &timer_callback);

    uint32_t divisor = 1193182 / frequency;

    outb(0x43, 0x36);
    outb(0x40, divisor & 0xff);
    outb(0x40, (divisor >> 8) & 0xff);
}

void int30(const struct registers_t* regs) {
    kprintf("An interrupt was caught with the following registers:\n");
    kprintf("ds: %x\n", regs->ds & 0xffff);
    kprintf("edi: %x\n", regs->edi);
    kprintf("esi: %x\n", regs->esi);
    kprintf("ebp: %x\n", regs->ebp);
    kprintf("esp: %x\n", regs->esp);
    kprintf("ebx: %x\n", regs->ebx);
    kprintf("edx: %x\n", regs->edx);
    kprintf("ecx: %x\n", regs->ecx);
    kprintf("eax: %x\n", regs->eax);
    kprintf("interrupt_number: %x\n", regs->interrupt_number);
    kprintf("error_code: %x\n", regs->error_code);
    kprintf("eip: %x\n", regs->eip);
    kprintf("cs: %x\n", regs->cs & 0xffff);
    kprintf("eflags: %x\n", regs->eflags);
    kprintf("user_esp: %x\n", regs->user_esp);
    kprintf("ss: %x\n", regs->ss & 0xffff);
}

extern void init_pic8259();
extern void init_paging(uint32_t max_memory);

int kmain() {
    init_pic8259();
    init_paging(32*1024*1024);

    kprintf("THIS IS MY AWESOME KERNEL\n");
    kprintf("AUTHOR: MARRONY N. NERIS\n");
    kprintf("VERSION: 1.0\n\n");

    //register_interrupt_handler(30, &int30);
    //__asm__ __volatile__ ("int $30");

    init_timer(19);

    while(1)
        hlt();

    return 0;
}

