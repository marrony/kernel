.extern kmain
.extern init_descriptor
.extern idt_ptr
.extern gdt_ptr
.extern tss_ptr

#define STACK_SIZE 0x8000

.globl start
start:
    movl $(__stack + STACK_SIZE), %esp
    call kmain
loop:
    hlt
    jmp loop

.globl __stack
.align 32 
.comm __stack, STACK_SIZE 


