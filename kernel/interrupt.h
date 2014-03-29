#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "context.h"

typedef void (*interrupt_handler_t)(interrupt_frame_t* frame);

#define IRQ0 32

void register_interrupt_handler(int interrupt, interrupt_handler_t callback);

void init_interrupt_controller();

#endif //INTERRUPT_H

