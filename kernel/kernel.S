.extern main

#define STACK_SIZE 0x8000

.globl load_gdt
.globl load_idt

start:
    movl $(stack + STACK_SIZE), %esp
    call main
loop:
    hlt
    jmp loop

load_gdt:
    mov 4(%esp), %eax
    lgdt (%eax)

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    
    ljmp $0x08, $load_gdt_out 
load_gdt_out:
    ret

load_idt:
    mov 4(%esp), %eax
    lidt (%eax)
    ret

.comm stack, STACK_SIZE 

