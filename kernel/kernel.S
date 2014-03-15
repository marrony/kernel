.extern kmain

#define STACK_SIZE 0x8000

.globl start
start:
    movl $(stack + STACK_SIZE), %esp
    call kmain
loop:
    cli
    hlt
    jmp loop

.comm stack, STACK_SIZE 

