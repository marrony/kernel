.extern main

start:
    movl $(stack + 0x4000), %esp
    call main

loop:
    hlt
    jmp loop

.comm stack, 0x4000

