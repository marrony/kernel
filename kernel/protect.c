#include <stdint.h>
#include <string.h>
#include "protect.h"
#include "asm.h"

//access flags
#define ACCESSED    (1 << 0)
#define READ        (1 << 1)
#define WRITE       (1 << 1)
#define EXPANSION   (1 << 2)
#define CONFORMING  (1 << 2)
#define CODE        (3 << 3)
#define DATA        (2 << 3)
#define DPL(x)      ((x) << 5)
#define PRESENT     (1 << 7)

//granularity flags
#define AVAILABLE   (1 << 4)
#define DEFAULT     (1 << 6)
#define BIG         (1 << 6)
#define GRANULAR    (1 << 7)

struct gdt_descriptor_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct idt_descriptor_t {
    uint16_t offset_low;
    uint16_t segment;
    uint8_t  reserved_zeros;
    uint8_t  privilege;
    uint16_t offset_high;
} __attribute__((packed));

struct table_pointer_t {
    uint16_t limit;
    uint32_t address;
} __attribute__((packed));

static struct gdt_descriptor_t gdt_table[5];

static void set_gdt_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt_table[index].base_low = base & 0xffff;
    gdt_table[index].base_middle = (base >> 16) & 0xff;
    gdt_table[index].base_high = (base >> 24) & 0xff;
    gdt_table[index].limit_low = limit & 0xffff;
    gdt_table[index].granularity = granularity | (limit >> 16);
    gdt_table[index].access = access; 
}

static void code_segment(int index, uint32_t base, uint32_t limit, uint8_t level) {
    uint8_t access = CODE | READ | PRESENT |  DPL(level);
    uint8_t granularity = DEFAULT | GRANULAR;
    set_gdt_entry(index, base, limit, access, granularity);
}

static void data_segment(int index, uint32_t base, uint32_t limit, uint8_t level) {
    uint8_t access = DATA | WRITE | PRESENT |  DPL(level);
    uint8_t granularity = DEFAULT | GRANULAR;
    set_gdt_entry(index, base, limit, access, granularity);
}

void init_gdt() {
    set_gdt_entry(0, 0, 0, 0, 0);
    code_segment(1, 0, 0xffffff, 0);
    data_segment(2, 0, 0xffffff, 0);
    code_segment(3, 0, 0xffffff, 3);
    data_segment(4, 0, 0xffffff, 3);

    struct table_pointer_t gdt_ptr = {
        .limit = sizeof(gdt_table) - 1,
        .address = (uint32_t)gdt_table
    };

    __asm__ __volatile__ (
    "    lgdt %0           \n"
    "    movw %1, %%ax     \n"
    "    movw %%ax, %%ds   \n"
    "    movw %%ax, %%es   \n"
    "    movw %%ax, %%fs   \n"
    "    movw %%ax, %%gs   \n"
    "    movw %%ax, %%ss   \n"
    "    ljmp %2, $1f      \n"
    "1:                    \n"
    : : "gm"(gdt_ptr), "i"(0x10), "i"(0x08));
}

///////////////////////////////////////////////////////////////

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

static void end_of_interrupt(int intrno) {
    if(intrno >= 0x28)
        outb(SLAVE_PIC_COMMAND, OCW2_EOI);

    outb(MASTER_PIC_COMMAND, OCW2_EOI);
}

static void remap_pic8259() {
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

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

static struct idt_descriptor_t idt_table[256];
static irq_callback_t irq_table[256];

static void set_idt_entry(int index, uint32_t base, uint16_t segment, uint8_t privilege) {
    idt_table[index].offset_low = base & 0xffff;
    idt_table[index].offset_high = (base >> 16) & 0xffff;
    idt_table[index].segment = segment;
    idt_table[index].privilege = privilege;
    idt_table[index].reserved_zeros = 0;
}

void init_idt() {
    remap_pic8259();

    memset(idt_table, 0, sizeof(idt_table));
    memset(irq_table, 0, sizeof(irq_table));

    set_idt_entry(0, (uint32_t)isr0, 0x08, 0x8e);
    set_idt_entry(1, (uint32_t)isr1, 0x08, 0x8e);
    set_idt_entry(2, (uint32_t)isr2, 0x08, 0x8e);
    set_idt_entry(3, (uint32_t)isr3, 0x08, 0x8e);
    set_idt_entry(4, (uint32_t)isr4, 0x08, 0x8e);
    set_idt_entry(5, (uint32_t)isr5, 0x08, 0x8e);
    set_idt_entry(6, (uint32_t)isr6, 0x08, 0x8e);
    set_idt_entry(7, (uint32_t)isr7, 0x08, 0x8e);
    set_idt_entry(8, (uint32_t)isr8, 0x08, 0x8e);
    set_idt_entry(9, (uint32_t)isr9, 0x08, 0x8e);
    set_idt_entry(10, (uint32_t)isr10, 0x08, 0x8e);
    set_idt_entry(11, (uint32_t)isr11, 0x08, 0x8e);
    set_idt_entry(12, (uint32_t)isr12, 0x08, 0x8e);
    set_idt_entry(13, (uint32_t)isr13, 0x08, 0x8e);
    set_idt_entry(14, (uint32_t)isr14, 0x08, 0x8e);
    set_idt_entry(15, (uint32_t)isr15, 0x08, 0x8e);
    set_idt_entry(16, (uint32_t)isr16, 0x08, 0x8e);
    set_idt_entry(17, (uint32_t)isr17, 0x08, 0x8e);
    set_idt_entry(18, (uint32_t)isr18, 0x08, 0x8e);
    set_idt_entry(19, (uint32_t)isr19, 0x08, 0x8e);
    set_idt_entry(20, (uint32_t)isr20, 0x08, 0x8e);
    set_idt_entry(21, (uint32_t)isr21, 0x08, 0x8e);
    set_idt_entry(22, (uint32_t)isr22, 0x08, 0x8e);
    set_idt_entry(23, (uint32_t)isr23, 0x08, 0x8e);
    set_idt_entry(24, (uint32_t)isr24, 0x08, 0x8e);
    set_idt_entry(25, (uint32_t)isr25, 0x08, 0x8e);
    set_idt_entry(26, (uint32_t)isr26, 0x08, 0x8e);
    set_idt_entry(27, (uint32_t)isr27, 0x08, 0x8e);
    set_idt_entry(28, (uint32_t)isr28, 0x08, 0x8e);
    set_idt_entry(29, (uint32_t)isr29, 0x08, 0x8e);
    set_idt_entry(30, (uint32_t)isr30, 0x08, 0x8e);
    set_idt_entry(31, (uint32_t)isr31, 0x08, 0x8e);

    set_idt_entry(32, (uint32_t)irq0, 0x08, 0x8e);
    set_idt_entry(33, (uint32_t)irq1, 0x08, 0x8e);
    set_idt_entry(34, (uint32_t)irq2, 0x08, 0x8e);
    set_idt_entry(35, (uint32_t)irq3, 0x08, 0x8e);
    set_idt_entry(36, (uint32_t)irq4, 0x08, 0x8e);
    set_idt_entry(37, (uint32_t)irq5, 0x08, 0x8e);
    set_idt_entry(38, (uint32_t)irq6, 0x08, 0x8e);
    set_idt_entry(39, (uint32_t)irq7, 0x08, 0x8e);
    set_idt_entry(40, (uint32_t)irq8, 0x08, 0x8e);
    set_idt_entry(41, (uint32_t)irq9, 0x08, 0x8e);
    set_idt_entry(42, (uint32_t)irq10, 0x08, 0x8e);
    set_idt_entry(43, (uint32_t)irq11, 0x08, 0x8e);
    set_idt_entry(44, (uint32_t)irq12, 0x08, 0x8e);
    set_idt_entry(45, (uint32_t)irq13, 0x08, 0x8e);
    set_idt_entry(46, (uint32_t)irq14, 0x08, 0x8e);
    set_idt_entry(47, (uint32_t)irq15, 0x08, 0x8e);

    struct table_pointer_t idt_ptr = {
        .limit = sizeof(idt_table) - 1,
        .address = (uint32_t)idt_table
    };

    __asm__ __volatile__ (
        "lidt %0"
        : : "gm"(idt_ptr)
    );
}

#include "kprintf.h"

void isr_handler(const struct registers_t regs) {
    kprintf("An interrupt was caught with the following registers:\n");
    kprintf("ds: %x\n", regs.ds & 0xffff);
    kprintf("edi: %x\n", regs.edi);
    kprintf("esi: %x\n", regs.esi);
    kprintf("ebp: %x\n", regs.ebp);
    kprintf("esp: %x\n", regs.esp);
    kprintf("ebx: %x\n", regs.ebx);
    kprintf("edx: %x\n", regs.edx);
    kprintf("ecx: %x\n", regs.ecx);
    kprintf("eax: %x\n", regs.eax);
    kprintf("interrupt_number: %x\n", regs.interrupt_number);
    kprintf("error_code: %x\n", regs.error_code);
    kprintf("eip: %x\n", regs.eip);
    kprintf("cs: %x\n", regs.cs & 0xffff);
    kprintf("eflags: %x\n", regs.eflags);
    kprintf("user_esp: %x\n", regs.user_esp);
    kprintf("ss: %x\n", regs.ss & 0xffff);
}

void register_irq(int intr, irq_callback_t callback) {
    irq_table[intr] = callback;
}

void irq_handler(const struct registers_t regs) {
    int intrno = regs.interrupt_number;
    
    end_of_interrupt(intrno);

    irq_callback_t handler = irq_table[intrno];
    if(handler != 0)
        handler(&regs);
}

void init_protect() {
    cli();
    init_gdt();
    init_idt();
    sti();
}

