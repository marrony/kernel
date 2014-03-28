#include "context.h"
#include "interrupt.h"

void system_fork(interrupt_frame_t* trap);

void system_call(interrupt_frame_t* trap) {
    system_fork(trap); //only system call
}

void init_system_call() {
    register_interrupt_handler(0x80, &system_call);
}
