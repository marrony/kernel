#include <stdint.h>
#include <string.h>

#include "interrupt.h"
#include "paging.h"
#include "task.h"
#include "global.h"
#include "asm.h"

#define MASTER_PIC         0x20
#define MASTER_PIC_COMMAND MASTER_PIC
#define MASTER_PIC_DATA    (MASTER_PIC+1)

#define SLAVE_PIC          0xa0
#define SLAVE_PIC_COMMAND  SLAVE_PIC
#define SLAVE_PIC_DATA     (SLAVE_PIC+1)

#define ICW1_INIT          0x10  // init=1
#define ICW1_LEVEL         0x08  // level=1, edge=0
#define ICW1_INTERVAL      0x04  // call address interval 4=1, 8=0
#define ICW1_SINGLE        0x02  // single=1, cascade=0
#define ICW1_ICW4          0x01  // need icw4=1, no need icw4=0

#define ICW2(x)            (x) 
#define ICW3_IRQ(x)        (1 << (x))
#define ICW3_SLAVE(x)      (x)
#define ICW4_8086          0x01  // 8086/8088=1, MCS-80/85=0
#define ICW4_AUTOEOI       0x02
#define ICW4_BUFF_MASTER   0x0c
#define ICW4_BUFF_SLAVE    0x08
#define ICW4_SFNM          0x10  // fully nested=1, not fully nested=0

#define OCW2_EOI           0x20

static interrupt_handler_t interrupt_table[256];

// initialize Programmable Interrupt Controller (i8259)
void init_interrupt_controller() {
    memset(interrupt_table, 0, sizeof(interrupt_table));

    uint8_t mask0 = inb(MASTER_PIC_DATA);
    uint8_t mask1 = inb(SLAVE_PIC_DATA);

    outb(MASTER_PIC_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(SLAVE_PIC_COMMAND, ICW1_INIT | ICW1_ICW4);

    outb(MASTER_PIC_DATA, ICW2(0x20));
    outb(SLAVE_PIC_DATA, ICW2(0x28));

    outb(MASTER_PIC_DATA, ICW3_IRQ(2));
    outb(SLAVE_PIC_DATA, ICW3_SLAVE(2));

    outb(MASTER_PIC_DATA, ICW4_8086);
    outb(SLAVE_PIC_DATA, ICW4_8086);

    outb(MASTER_PIC_DATA, mask0);
    outb(SLAVE_PIC_DATA, mask1);
}

void set_interrupt_handler(int intr, interrupt_handler_t callback) {
    interrupt_table[intr] = callback;
}

static void end_of_interrupt(int intrno) {
    if(intrno >= 0x28)
        outb(SLAVE_PIC_COMMAND, OCW2_EOI);

    outb(MASTER_PIC_COMMAND, OCW2_EOI);
}

void handle_interrupt(struct trap_t* trap) {
    current_task->trap = trap;

    int intrno = trap->interrupt_number;
    
    end_of_interrupt(intrno);

    interrupt_handler_t handler = interrupt_table[intrno];
    if(handler != 0)
        handler(trap);
}

