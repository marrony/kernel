#include "syscall.h"
#include "context.h"
#include "interrupt.h"

void system_fork(interrupt_frame_t*);
void system_getpid(interrupt_frame_t*);

static void (*system_calls[])(interrupt_frame_t*) = {
    [SYSTEM_fork]   system_fork,
    [SYSTEM_getpid] system_getpid,
};

void system_call(interrupt_frame_t* trap) {
    int syscall_number = trap->eax;

    system_calls[syscall_number](trap);
}

void init_system_call() {
    register_interrupt_handler(0x80, &system_call);
}

