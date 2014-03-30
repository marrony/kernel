#include "syscall.h"
#include "task.h"
#include "interrupt.h"
#include "context.h"

void system_fork();
void system_getpid();

static void (*system_calls[])() = {
    [SYSTEM_fork]   system_fork,
    [SYSTEM_getpid] system_getpid,
};

void system_call() {
    int syscall_number = current_task->trap->eax;

    system_calls[syscall_number]();
}

void init_system_call() {
    register_interrupt_handler(0x80, &system_call);
}

