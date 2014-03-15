.globl load_gdt
.globl load_idt

load_gdt:
    movl 4(%esp), %eax
    lgdt (%eax) 
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ljmp $0x08, $1f
1:
    ret

load_idt:
    movl 4(%esp), %eax
    lidt (%eax) 
    ret

