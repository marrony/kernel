#ifndef INTERRUPT_H
#define INTERRUPT_H

typedef void (*interrupt_handler_t)();

void set_interrupt_handler(int interrupt, interrupt_handler_t callback);

#define IRQ0  32           // Timer
#define IRQ1  (IRQ0 + 1)   // Keyboard
#define IRQ2  (IRQ0 + 2)   // Cascade for 8259A Slave controller
#define IRQ3  (IRQ0 + 3)   // Serial port 2
#define IRQ4  (IRQ0 + 4)   // Serial port 1
#define IRQ5  (IRQ0 + 5)   // AT systems: Parallel Port 2. PS/2 systems: reserved
#define IRQ6  (IRQ0 + 6)   // Diskette drive
#define IRQ7  (IRQ0 + 7)   // Parallel Port 1
#define IRQ8  (IRQ0 + 8)   // CMOS Real time clock
#define IRQ9  (IRQ0 + 9)   // CGA vertical retrace
#define IRQ10 (IRQ0 + 10)  // Reserved
#define IRQ11 (IRQ0 + 11)  // Reserved
#define IRQ12 (IRQ0 + 12)  // AT systems: reserved. PS/2: auxiliary device
#define IRQ13 (IRQ0 + 13)  // FPU
#define IRQ14 (IRQ0 + 14)  // Hard disk controller
#define IRQ15 (IRQ0 + 15)  // Reserved

#endif //INTERRUPT_H

