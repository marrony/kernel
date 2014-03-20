.extern kmain
.extern init_desc
.extern idt_ptr
.extern gdt_ptr

#define STACK_SIZE 0x8000

.globl start
start:
    movl $(__stack + STACK_SIZE), %esp

    call init_desc
    lidt idt_ptr
    lgdt gdt_ptr

    ljmp $0x08, $1f
1:  movw $0x10, %ax

    cli
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    sti

    call kmain

loop:
    hlt
    jmp loop

.globl __stack
.align 4096
.comm __stack, STACK_SIZE 


