#ifndef TASK_H
#define TASK_H

struct context_t;
struct interrupt_frame_t;
struct pde_t;

typedef struct task_t {
    int pid;
    struct interrupt_frame_t* trap; 
    struct context_t* context;
    void* stack;
    struct pde_t* page_directory;
    struct task_t* next;
} task_t;

void init_tasking();

extern struct task_t* current_task;

#endif //TASK_H

