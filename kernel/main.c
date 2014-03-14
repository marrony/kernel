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

#define cli() __asm__ __volatile__("cli")
#define sti() __asm__ __volatile__("sti")

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

void init_idt_table() {
    memset(idt_table, 0, sizeof(idt_table));

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

    load_idt(&idt_ptr);
}

void interrupt_handler(const struct registers_t regs) {
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

int main() {
    init_gdt_table();
    init_idt_table();

    puts("THIS IS MY AWESOME KERNEL\n");
    puts("AUTHOR: MARRONY N. NERIS\n");
    puts("VERSION: 1.0\n\n");
   
    cli();
    __asm__ __volatile__ ("int $0x04");

    return 0;
}
