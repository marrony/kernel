#include "inttypes.h"
#include "stdarg.h"
#include "stdio.h"
#include "io.h"

int16_t* video_memory = (int16_t*)0xb8000;
int16_t video_x_position = 0;
int16_t video_y_position = 0;
int16_t video_attribute = 0x0f00;
int16_t video_width = 80;
int16_t video_height = 25;
int8_t video_tab_size = 8;

void memset(void* ptr, int byte, size_t size) {
    char* p = (char*)ptr;
    for(size_t i = 0; i < size; i++)
        *p++ = byte;
}

#define LINEAR(x, y) ((y)*video_width + (x))

void gotoxy() {
    int16_t location = LINEAR(video_x_position, video_y_position);
    outb(0x3d4, 14);
    outb(0x3d5, (location >> 8) & 0xff);
    outb(0x3d4, 15);
    outb(0x3d5, location & 0xff);
}

void putch(char ch) {
    if(ch == 0x08 && video_x_position > 0) {
        video_x_position--;
    } else if(ch == 0x09) {
        video_x_position = (video_x_position + video_tab_size) & ~(video_tab_size - 1);
    } else if(ch == '\r') {
        video_x_position = 0;
    } else if(ch == '\n') {
        video_x_position = 0;
        video_y_position++;
    } else {
        int16_t linear = LINEAR(video_x_position, video_y_position);
        video_memory[linear] = video_attribute | ch;
        video_x_position++;
    }

    if(video_x_position >= video_width) {
        video_x_position = 0;
        video_y_position++;
    }

    if(video_y_position >= video_height) {
        int16_t empty = video_attribute | ' ';

        for(int line = 0; line < video_height-1; line++) {
            int16_t* line0 = video_memory + LINEAR(0, line+0); 
            int16_t* line1 = video_memory + LINEAR(0, line+1);

            for(int x = 0; x < video_width; x++)
                line0[x] = line1[x];
        }

        int16_t* last_line = video_memory + LINEAR(0, video_height-1);
        for(int x = 0; x < video_width; x++)
            last_line[x] = empty;

        video_y_position--;
    }

    gotoxy();
}

void puts(const char* str) {
    while(*str) {
        putch(*str++);
    }
}

int printf(const char* fmt, ...) {
    char buffer[1024 + 1];
    va_list va;
    int ret;

    va_start(va, fmt);
    ret = vsnprintf(buffer, sizeof(buffer) - 1, fmt, va);
    va_end(va);

    puts(buffer);
    return ret;
}

struct gdt_descriptor {
    uint32_t segment_limit_low : 16;
    uint32_t base_low : 16;
    uint32_t base_middle : 8;
    uint32_t privilege : 8;
    uint32_t segment_limit_high : 4;
    uint32_t granularity : 4;
    uint32_t base_high : 8;
} __attribute__((packed));

struct idt_interrupt_descriptor {
    uint32_t offset_low : 16;
    uint32_t segment : 16;
    uint32_t reserved_zeros : 8;
    uint32_t privilege : 8;
    uint32_t offset_high : 16;
} __attribute__((packed));

struct table_pointer {
    uint16_t limit;
    uint32_t address;
} __attribute__((packed));

struct gdt_descriptor gdt_table[5];
struct idt_interrupt_descriptor idt_table[256];
struct table_pointer gdt_ptr;
struct table_pointer idt_ptr;

void set_gdt_entry(int index, uint32_t base, uint32_t limit, int privilege, int granularity) {
    gdt_table[index].base_low = base & 0xffff;
    gdt_table[index].base_middle = (base >> 16) & 0xff;
    gdt_table[index].base_high = (base >> 24) & 0xff;
    gdt_table[index].segment_limit_low = limit & 0xffff;
    gdt_table[index].segment_limit_high = (limit >> 16) & 0x0f;
    gdt_table[index].privilege = privilege & 0xff;
    gdt_table[index].granularity = granularity & 0x0f;
}

void set_idt_entry(int index, uint32_t base, uint32_t segment, int privilege) {
    idt_table[index].offset_low = base & 0xffff;
    idt_table[index].offset_high = (base >> 16) & 0xffff;
    idt_table[index].segment = segment & 0xffff;
    idt_table[index].privilege = privilege & 0xff;
    idt_table[index].reserved_zeros = 0;
}

extern void load_gdt(struct table_pointer* ptr);
extern void load_idt(struct table_pointer* ptr);

void init_gdt_table() {
    set_gdt_entry(0, 0, 0, 0, 0);
    set_gdt_entry(1, 0, 0xffffffff, 0x9a, 0x0c);
    set_gdt_entry(2, 0, 0xffffffff, 0x92, 0x0c);
    set_gdt_entry(3, 0, 0xffffffff, 0xfa, 0x0c);
    set_gdt_entry(4, 0, 0xffffffff, 0xf2, 0x0c);

    gdt_ptr.limit = sizeof(gdt_table) - 1;
    gdt_ptr.address = (uint32_t)&gdt_table;

    load_gdt(&gdt_ptr);
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

#define MASTER_PIC         0x20
#define MASTER_PIC_COMMAND MASTER_PIC
#define MASTER_PIC_DATA    (MASTER_PIC+1)

#define SLAVE_PIC         0xa0
#define SLAVE_PIC_COMMAND SLAVE_PIC
#define SLAVE_PIC_DATA    (SLAVE_PIC+1)

#define ICW1_INIT      0x10  // init=1
#define ICW1_LEVEL     0x08  // level=1, edge=0
#define ICW1_INTERVAL  0x04  // call address interval 4=1, 8=0
#define ICW1_SINGLE    0x02  // single=1, cascade=0
#define ICW1_ICW4      0x01  // need icw4=1, no need icw4=0

#define ICW2(x)          (x) 
#define ICW3_IRQ(x)      (1 << (x))
#define ICW3_SLAVE(x)    (x)
#define ICW4_8086        0x01  // 8086/8088=1, MCS-80/85=0
#define ICW4_AUTOEOI     0x02
#define ICW4_BUFF_MASTER 0x0c
#define ICW4_BUFF_SLAVE  0x08
#define ICW4_SFNM        0x10  // fully nested=1, not fully nested=0

#define OCW2_EOI 0x20

void end_of_interrupt(int intrno) {
    if(intrno >= 0x28)
        outb(SLAVE_PIC_COMMAND, OCW2_EOI);

    outb(MASTER_PIC_COMMAND, OCW2_EOI);
}

void remap_pic8259() {
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

#define IRQ0 32

typedef void (*interrupt_callback_t)(const struct registers_t* regs);

interrupt_callback_t interrupt_callback[256];

void irq_handler(const struct registers_t regs) {
    int intrno = regs.interrupt_number;
    
    end_of_interrupt(intrno);

    interrupt_callback_t handler = interrupt_callback[intrno];
    if(handler != 0)
        handler(&regs);
}

void init_idt_table() {
    remap_pic8259();

    memset(idt_table, 0, sizeof(idt_table));
    memset(interrupt_callback, 0, sizeof(interrupt_callback));

    idt_ptr.limit = sizeof(idt_table) - 1;
    idt_ptr.address = (uint32_t)&idt_table;

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
    
    load_idt(&idt_ptr);
}

void isr_handler(const struct registers_t regs) {
    printf("An interrupt was caught with the following registers:\n");
    printf("ds: %x\n", regs.ds & 0xffff);
    printf("edi: %x\n", regs.edi);
    printf("esi: %x\n", regs.esi);
    printf("ebp: %x\n", regs.ebp);
    printf("esp: %x\n", regs.esp);
    printf("ebx: %x\n", regs.ebx);
    printf("edx: %x\n", regs.edx);
    printf("ecx: %x\n", regs.ecx);
    printf("eax: %x\n", regs.eax);
    printf("interrupt_number: %x\n", regs.interrupt_number);
    printf("error_code: %x\n", regs.error_code);
    printf("eip: %x\n", regs.eip);
    printf("cs: %x\n", regs.cs & 0xffff);
    printf("eflags: %x\n", regs.eflags);
    printf("user_esp: %x\n", regs.user_esp);
    printf("ss: %x\n", regs.ss & 0xffff);
}

void timer_callback(const struct registers_t* regs) {
    static uint32_t ticks = 0;

    ticks++;
    printf("\rclock ticks: %d", ticks);
}

void init_timer(int frequency) {
    interrupt_callback[IRQ0] = timer_callback;

    uint32_t divisor = 1193180 / frequency;

    outb(0x43, 0x36);
    outb(0x40, divisor & 0xff);
    outb(0x40, (divisor >> 8) & 0xff);
}

#define cli() __asm__ __volatile__("cli")
#define sti() __asm__ __volatile__("sti")
#define hlt() __asm__ __volatile__("hlt")

int main() {
    cli();
    init_gdt_table();
    init_idt_table();
    sti();

    puts("THIS IS MY AWESOME KERNEL\n");
    puts("AUTHOR: MARRONY N. NERIS\n");
    puts("VERSION: 1.0\n\n");

    init_timer(18);

    while(1)
        hlt();

    return 0;
}
