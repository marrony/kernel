#include <stdint.h>
#include "asm.h"
#include "irq.h"
#include "regs.h"
#include "task.h"
#include "kprintf.h"

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

int fork() {
    int ret;
    __asm__ __volatile__ (
    "   int $0x80  \n"
    : "=a"(ret)
    );
    return ret;
}

int kmain() {
    init_pic8259();
    init_paging(32*1024*1024);
    init_tasking();

    int pid;
    if(!(pid = fork())) {
        kprintf("THIS IS MY AWESOME KERNEL\n");
        kprintf("AUTHOR: MARRONY N. NERIS\n");
        kprintf("VERSION: 1.0\n\n");
        static int count = 0;
        while(1) {
            kprintf("main %d\n", count++);
            hlt();
        }
    } else {
        while(1) {
            kprintf("Hi, I'm a child with pid: %d\n", pid);
            hlt();
        }
    }

    return 0;
}

