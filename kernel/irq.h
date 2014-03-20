#ifndef IRQ_H
#define IRQ_H

#include "regs.h"

typedef void (*interrupt_handler_t)(const struct registers_t* regs);

#define IRQ0 32

void register_interrupt_handler(int interrupt, interrupt_handler_t callback);

#endif //IRQ_H

