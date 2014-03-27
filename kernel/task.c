#include "task.h"

#include "asm.h"
#include "irq.h"
#include "paging.h"
#include "heap.h"

#include <string.h>

typedef struct task_t {
    int pid;
    registers_t* trap; 
    context_t* context;
    void* stack;
    page_directory_t* page_directory;
    struct task_t* next;
} task_t;

volatile task_t* ready_queue_start = 0;
volatile task_t* ready_queue_end = 0;
volatile task_t* current_task = 0;

int next_pid = 0;
uint32_t ticks = 0;
context_t kernel_context;

__asm__ (
".globl load_eip \n"
"load_eip:       \n"
"    popl %eax   \n"
"    jmp %eax    \n"
);

uint32_t load_eip();

/*
  +=========+
  |  ss     |\   <- pushed only if a privelege level change occurs
  +=========+ \
  |  esp    |  \
  +=========+    int/iret
  |  eflgs  |  /
  +=========+ /
  |  cs     |/
  +=========+
  |  eip    |
  +=========+
  |  error  |\
  +=========+  add esp,8
  |  int nr |/
  +=========+
  |  eax    |\
  +=========+ \
  |  ecx    |  \
  +=========+
  |  edx    |
  +=========+
  |  ebx    |
  +=========+     pusha/popa
  |  esp    |
  +=========+
  |  ebp    |
  +=========+
  |  esi    |   /
  +=========+  /
  |  edi    | /
  +=========+
  |  ds     |
  +=========+
  |         |  isr_handler
  +=========+
  |         |  handler
  +=========+
  |         |  timer_callback
  +=========+
  |         |  schedule
  +=========+  <- esp

*/

extern void irq_end();
#include "kprintf.h"
void fork_exit() {
    kprintf("fork_exit");
}

void system_fork(registers_t* regs) {
    page_directory_t* page_directory = current_directory;
    
    task_t* new_task = (task_t*)kmalloc(sizeof(task_t));
    memset(new_task, 0, sizeof(task_t));

    new_task->pid = next_pid++;
    new_task->page_directory = page_directory;
    new_task->stack = kmalloc(4096);
    memset(new_task->stack, 0, 4096);

    uint32_t esp = (uint32_t)new_task->stack + 4096;

    esp -= sizeof(registers_t);
    new_task->trap = (registers_t*)esp;

    esp -= sizeof(context_t);
    new_task->context = (context_t*)esp;
    memset(new_task->context, 0, sizeof(context_t));
    new_task->context->eip = (uint32_t) irq_end;

    memcpy(new_task->trap, regs, sizeof(registers_t));
    new_task->trap->eax = 0;

    regs->eax = new_task->pid;

    ready_queue_end->next = new_task;
    ready_queue_end = new_task;
}

void system_call(registers_t* regs) {
    system_fork(regs); //only system call
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

void switch_context(context_t** old, context_t* new);

void schedule() {
    task_t* old_task = (task_t*)current_task;

    current_task = current_task->next;

    if(!current_task) 
        current_task = ready_queue_start;

    switch_context(&old_task->context, current_task->context);
}

void timer_callback(registers_t* regs) {
    ticks++;
    current_task->trap = regs;
    schedule();
}

void init_timer(uint32_t frequency) {
    register_interrupt_handler(IRQ0, &timer_callback);

    uint32_t divisor = 1193182 / frequency;

    outb(0x43, 0x36);
    outb(0x40, divisor & 0xff);
    outb(0x40, (divisor >> 8) & 0xff);
}

void init_tasking() {
    current_task = (task_t*)kmalloc(sizeof(task_t));
    current_task->pid = next_pid++;
    current_task->page_directory = current_directory;
    current_task->next = 0;

    ready_queue_start = current_task;
    ready_queue_end = current_task;

    register_interrupt_handler(0x80, &system_call);

    init_timer(19);
}

int getpid() {
    return current_task->pid;
}

