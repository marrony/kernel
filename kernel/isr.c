#include "regs.h"
#include "kprintf.h"

void isr_handler(const struct registers_t regs) {
    kprintf("An interrupt was caught with the following registers:\n");
    kprintf("ds: %x\n", regs.ds & 0xffff);
    kprintf("edi: %x\n", regs.edi);
    kprintf("esi: %x\n", regs.esi);
    kprintf("ebp: %x\n", regs.ebp);
    kprintf("esp: %x\n", regs.esp);
    kprintf("ebx: %x\n", regs.ebx);
    kprintf("edx: %x\n", regs.edx);
    kprintf("ecx: %x\n", regs.ecx);
    kprintf("eax: %x\n", regs.eax);
    kprintf("interrupt_number: %x\n", regs.interrupt_number);
    kprintf("error_code: %x\n", regs.error_code);
    kprintf("eip: %x\n", regs.eip);
    kprintf("cs: %x\n", regs.cs & 0xffff);
    kprintf("eflags: %x\n", regs.eflags);
    kprintf("user_esp: %x\n", regs.user_esp);
    kprintf("ss: %x\n", regs.ss & 0xffff);
}

