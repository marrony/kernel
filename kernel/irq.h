#ifndef IRQ_H
#define IRQ_H

#include "regs.h"

typedef void (*irq_callback_t)(const struct registers_t* regs);

#define IRQ0 32

void register_irq(int irq, irq_callback_t callback);

#endif //IRQ_H

