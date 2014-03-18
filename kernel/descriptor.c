#include <stdint.h>
#include <string.h>
#include "intrs.h"
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
struct table_pointer_t gdt_ptr;

static struct idt_descriptor_t idt_table[256];
struct table_pointer_t idt_ptr;

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

static void init_gdt() {
    set_gdt_entry(0, 0, 0, 0, 0);
    code_segment(1, 0, 0xffffff, 0);
    data_segment(2, 0, 0xffffff, 0);
    code_segment(3, 0, 0xffffff, 3);
    data_segment(4, 0, 0xffffff, 3);

    gdt_ptr.limit = sizeof(gdt_table) - 1;
    gdt_ptr.address = (uint32_t)gdt_table;
}

static void set_idt_entry(int index, uint32_t base, uint16_t segment, uint8_t privilege) {
    idt_table[index].offset_low = base & 0xffff;
    idt_table[index].offset_high = (base >> 16) & 0xffff;
    idt_table[index].segment = segment;
    idt_table[index].privilege = privilege;
    idt_table[index].reserved_zeros = 0;
}

static void init_idt() {
    memset(idt_table, 0, sizeof(idt_table));

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

    idt_ptr.limit = sizeof(idt_table) - 1;
    idt_ptr.address = (uint32_t)idt_table;
}

void init_desc() {
    init_gdt();
    init_idt();
}

