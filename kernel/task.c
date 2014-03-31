#include <stdint.h>
#include <string.h>

#include "task.h"
#include "kernel.h"
#include "asm.h"
#include "interrupt.h"
#include "paging.h"
#include "heap.h"
#include "global.h"

struct task_t* current_task;
static struct task_t* ready_queue_start = 0;
static struct task_t* ready_queue_end = 0;

static int next_pid = 0;
static uint32_t ticks = 0;

extern void trap_end();

void system_fork() {
    struct pde_t* page_directory = current_directory;
    
    struct task_t* new_task = (struct task_t*)kmalloc(sizeof(struct task_t));
    memset(new_task, 0, sizeof(struct task_t));

    new_task->pid = next_pid++;
    new_task->page_directory = page_directory;
    new_task->stack = kmalloc(4096);
    memset(new_task->stack, 0, 4096);

    uint32_t esp = (uint32_t)new_task->stack + 4096;

    esp -= sizeof(struct trap_t);
    new_task->trap = (struct trap_t*)esp;

    esp -= sizeof(struct context_t);
    new_task->context = (struct context_t*)esp;
    memset(new_task->context, 0, sizeof(struct context_t));
    new_task->context->eip = (uint32_t) trap_end;

    memcpy(new_task->trap, current_task->trap, sizeof(struct trap_t));

    new_task->trap->eax = 0;
    current_task->trap->eax = new_task->pid;

    ready_queue_end->next = new_task;
    ready_queue_end = new_task;
}

void system_getpid() {
    current_task->trap->eax = current_task->pid;
}

__asm__ (
".globl switch_context     \n"
"switch_context:           \n"
"    movl 4(%esp), %eax    \n"  //old
"    movl 8(%esp), %edx    \n"  //new
"                          \n"
"    pushl %ebp            \n"
"    pushl %eax            \n"
"    pushl %ebx            \n"
"    pushl %ecx            \n"
"    pushl %edx            \n"
"    pushl %esi            \n"
"    pushl %edi            \n"
"                          \n"
"    # Switch stacks       \n"
"    movl %esp, (%eax)     \n"
"    movl %edx, %esp       \n"
"                          \n"
"    popl %edi             \n"
"    popl %esi             \n"
"    popl %edx             \n"
"    popl %ecx             \n"
"    popl %ebx             \n"
"    popl %eax             \n"
"    popl %ebp             \n"
"    ret                   \n"
);

void switch_context(struct context_t** old, struct context_t* new);

void schedule() {
    struct task_t* old_task = current_task;

    current_task = current_task->next;

    if(!current_task)
        current_task = ready_queue_start;

    if(current_task != old_task)
        switch_context(&old_task->context, current_task->context);
}

void timer_callback() {
    ticks++;
    schedule();
}

void init_tasking() {
    current_task = (struct task_t*)kmalloc(sizeof(struct task_t));
    current_task->pid = next_pid++;
    current_task->page_directory = current_directory;
    current_task->next = 0;

    ready_queue_start = current_task;
    ready_queue_end = current_task;

    register_interrupt_handler(IRQ0, &timer_callback);

    init_timer(19);
}

