#ifndef GLOBAL_H
#define GLOBAL_H

// kernel.ld
extern const char __kernel_start[];
extern const char __kernel_end[];
extern const char __text_start[];
extern const char __text_end[];
extern const char __data_start[];
extern const char __data_end[];
extern const char __bss_start[];
extern const char __bss_end[];

// start.S
extern const char __stack[];

// paging.c
extern struct pde_t* kernel_directory;
extern struct pde_t* current_directory;

// task.c
extern struct task_t* current_task;

// trap.S
extern const uint32_t trap_vector[];

extern volatile uint32_t ticks;
#define HZ 19

#endif //GLOBAL_H
