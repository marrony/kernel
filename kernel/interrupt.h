#ifndef INTERRUPT_H
#define INTERRUPT_H

typedef void (*interrupt_handler_t)();

void register_interrupt_handler(int interrupt, interrupt_handler_t callback);

#define IRQ0 32

#endif //INTERRUPT_H

