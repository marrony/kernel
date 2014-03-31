#ifndef KERNEL_H
#define KERNEL_H

// descriptor.c
void init_descriptor();

// interrupt.c
void init_interrupt_controller();

// syscall.c
void init_system_call();

// timer.c
void init_timer(uint32_t frequency);

// task.c
void init_tasking();

// paging.c
void init_paging(uint32_t max_memory);

// kprintf.c
void kprintf(const char* fmt, ...);

#endif //KERNEL_H
