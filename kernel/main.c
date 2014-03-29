#include "asm.h"
#include "syscall.h"
#include "interrupt.h"
#include "paging.h"
#include "context.h"
#include "task.h"
#include "kprintf.h"

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

int kmain() {
    cli();
    init_interrupt_controller();
    init_paging(32*1024*1024);
    init_tasking();
    init_system_call();
    sti();

    int pid;
    if((pid = fork()) != 0) {
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
            kprintf("Hi, I'm a child with pid: %d\n", getpid());
            hlt();
        }
    }

    return 0;
}

