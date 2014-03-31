#include <stdint.h>

#include "asm.h"
#include "syscall.h"
#include "interrupt.h"
#include "kernel.h"

int fork() {
    int ret;
    __asm__ __volatile__ (
        "int $0x80" : "=a"(ret) : "a"(SYSTEM_fork)
    );
    return ret;
}

int getpid() {
    int ret;
    __asm__ __volatile__ (
        "int $0x80" : "=a"(ret) : "a"(SYSTEM_getpid)
    );
    return ret;
}

void keyboard() {
    uint8_t status = inb(0x64);
    uint8_t data = inb(0x60);
    kprintf("a key was pressed status: %d data: %d\n", status, data);
}

int kmain() {
    cli();
    init_descriptor();
    init_interrupt_controller();
    init_paging(32*1024*1024);
    init_tasking();
    init_system_call();
    register_interrupt_handler(IRQ0 + 1, &keyboard);
    sti();

    int pid;
    if((pid = fork()) != 0) {
        kprintf("THIS IS MY AWESOME KERNEL\n");
        kprintf("AUTHOR: MARRONY N. NERIS\n");
        kprintf("VERSION: 1.0\n\n");
        //static int count = 0;
        while(1) {
            //kprintf("main %d\n", count++);
            hlt();
        }
    } else {
        while(1) {
            //kprintf("Hi, I'm a child with pid: %d\n", getpid());
            hlt();
        }
    }

    return 0;
}

