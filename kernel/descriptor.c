#include <stdint.h>
#include <string.h>

#include "asm.h"
#include "global.h"

//access flags
#define ACCESSED    (1 << 0)
#define READ        (1 << 1)
#define WRITE       (1 << 1)
#define EXPANSION   (1 << 2)
#define CONFORMING  (1 << 2)
#define EXECUTABLE  (1 << 3)
#define CODE        ((1 << 4) | EXECUTABLE)
#define DATA        (1 << 4)
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

struct tss_descriptor_t {
    uint32_t previous_task;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap;
} __attribute__((packed));

struct table_pointer_t {
    uint16_t limit;
    uint32_t address;
} __attribute__((packed));

static struct gdt_descriptor_t gdt_table[6];
static struct idt_descriptor_t idt_table[256];
static struct tss_descriptor_t tss_entry;

static void set_gdt_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt_table[index].base_low = base & 0xffff;
    gdt_table[index].base_middle = (base >> 16) & 0xff;
    gdt_table[index].base_high = (base >> 24) & 0xff;
    gdt_table[index].limit_low = limit & 0xffff;
    gdt_table[index].granularity = granularity | (limit >> 16);
    gdt_table[index].access = access; 
}

static void code_segment(int index, uint32_t base, uint32_t limit, uint8_t level) {
    uint8_t access = CODE | READ | PRESENT | DPL(level);
    uint8_t granularity = DEFAULT | GRANULAR;
    set_gdt_entry(index, base, limit, access, granularity);
}

static void data_segment(int index, uint32_t base, uint32_t limit, uint8_t level) {
    uint8_t access = DATA | WRITE | PRESENT | DPL(level);
    uint8_t granularity = DEFAULT | GRANULAR;
    set_gdt_entry(index, base, limit, access, granularity);
}

static void tss_segment(int index, uint32_t base, uint32_t limit, uint32_t level) {
    uint8_t access = ACCESSED | PRESENT | EXECUTABLE | DPL(level);
    uint8_t granularity = BIG;
    set_gdt_entry(index, base, limit, access, granularity);
}

static void set_idt_entry(int index, uint32_t base, uint16_t segment, uint8_t privilege) {
    idt_table[index].offset_low = base & 0xffff;
    idt_table[index].offset_high = (base >> 16) & 0xffff;
    idt_table[index].segment = segment;
    idt_table[index].privilege = privilege;
    idt_table[index].reserved_zeros = 0;
}

static void init_gdt() {
    set_gdt_entry(0, 0, 0, 0, 0);
    code_segment(1, 0, 0xffffff, 0);
    data_segment(2, 0, 0xffffff, 0);
    code_segment(3, 0, 0xffffff, 3);
    data_segment(4, 0, 0xffffff, 3);
}

static void init_idt() {
    memset(idt_table, 0, sizeof(idt_table));

    for(int i = 0; i < 256; i++)
        set_idt_entry(i, trap_vector[i], 0x08, 0x8e);
}

static void init_tss() {
    uint32_t base = (uint32_t)&tss_entry;
    tss_segment(5, base, base+sizeof(struct tss_descriptor_t), 0);

    memset(&tss_entry, 0, sizeof(struct tss_descriptor_t));

    tss_entry.ss0 = 0x10;
    tss_entry.esp0 = 0;

    //set last 2 bits because this tss is for switch task from level 3 to level 0
    tss_entry.cs = 0x8 | 0x3;
    tss_entry.ss =
    tss_entry.es =
    tss_entry.ds =
    tss_entry.fs =
    tss_entry.gs = 0x10 | 0x3;
}

void init_descriptor() {
    init_gdt();
    init_idt();
    init_tss();

    struct table_pointer_t gdt_ptr; 
    gdt_ptr.limit = sizeof(gdt_table) - 1;
    gdt_ptr.address = (uint32_t)gdt_table;

    struct table_pointer_t idt_ptr;
    idt_ptr.limit = sizeof(idt_table) - 1;
    idt_ptr.address = (uint32_t)idt_table;

    uint16_t tss_ptr = 0x28 | 0x3;

    __asm__ __volatile__ ( 
    "    lidt %0              \n"
    "    lgdt %1              \n"
    "    ltr %2               \n"
    "                         \n"
    "    ljmp %3, $1f         \n"
    "1:  movw %4, %%ax        \n"
    "                         \n"
    "    movw %%ax, %%ds      \n"
    "    movw %%ax, %%es      \n"
    "    movw %%ax, %%fs      \n"
    "    movw %%ax, %%gs      \n"
    "    movw %%ax, %%ss      \n"
    : : "g"(idt_ptr),
        "g"(gdt_ptr),
        "r"(tss_ptr),
        "i"(0x08),
        "i"(0x10)
    );
}

