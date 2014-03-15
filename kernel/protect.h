#ifndef PROTECT_H
#define PROTECT_H

#include <stdint.h> 

struct registers_t {
    uint32_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t interrupt_number;
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t user_esp;
    uint32_t ss;
} __attribute__((packed));

typedef void (*irq_callback_t)(const struct registers_t* regs);

#define IRQ0 32

void init_protect();
void register_irq(int irq, irq_callback_t callback);

#endif //PROTECT_H

