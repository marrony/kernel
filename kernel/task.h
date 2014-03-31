#ifndef TASK_H
#define TASK_H

struct pde_t;

struct context_t {
    uint32_t edi;
    uint32_t esi;
    uint32_t edx;
    uint32_t ecx;
    uint32_t ebx;
    uint32_t eax;
    uint32_t ebp;
    uint32_t eip;
} __attribute__((packed));

struct trap_t {
    uint32_t ds;
    //pusha/popa
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp; //ignored
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    //interrupt values
    uint32_t interrupt_number;
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    //only valid when a privilege change occurs
    uint32_t user_esp;
    uint32_t ss;
} __attribute__((packed));

struct task_t {
    int pid;
    struct trap_t* trap; 
    struct context_t* context;
    void* stack;
    struct pde_t* page_directory;
    struct task_t* next;
};

#endif //TASK_H

