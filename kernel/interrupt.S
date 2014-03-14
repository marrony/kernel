.extern interrupt_handler

.macro ISR intno
.globl isr\intno
isr\intno:
    cli
    pushl $0x00
    pushl $\intno
    jmp isr_common
.endm

.macro ISR_ERROR intno 
.globl isr\intno
isr\intno:
    cli
    #already pushed
    pushl $\intno
    jmp isr_common
.endm

ISR 0
ISR 1
ISR 2
ISR 3
ISR 4
ISR 5
ISR 6
ISR 7
ISR_ERROR 8 
ISR 9
ISR_ERROR 10
ISR_ERROR 11 
ISR_ERROR 12
ISR_ERROR 13
ISR_ERROR 14
ISR 15 
ISR 16 
ISR 17 
ISR 18
ISR 19
ISR 20
ISR 21
ISR 22
ISR 23
ISR 24
ISR 25
ISR 26
ISR 27
ISR 28
ISR 29
ISR 30
ISR 31

/*
  Stack content at this moment

  +=========+
  |  ss     |   <- pushed only if a privelege level change occurs
  +=========+
  |  esp    |
  +=========+
  |  eflgs  |
  +=========+
  |  cs     |
  +=========+
  |  eip    |
  +=========+
  |  error  |
  +=========+
  |  int nr |
  +=========+   <- esp
*/
isr_common:
    pusha

    movw %ds, %ax
    pushl %eax

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    call interrupt_handler

    popl %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    popa
    addl $0x8, %esp

    sti
    iret

/*
  Stack content right before call interrupt_handler

  +=========+
  |  ss     |   <- pushed only if a privelege level change occurs
  +=========+
  |  esp    |
  +=========+
  |  eflgs  |
  +=========+
  |  cs     |
  +=========+
  |  eip    |
  +=========+
  |  error  |
  +=========+
  |  int nr |
  +=========+
  |  eax    |
  +=========+
  |  ecx    |
  +=========+
  |  edx    |
  +=========+
  |  ebx    |
  +=========+
  |  esp    |
  +=========+
  |  ebp    |
  +=========+
  |  esi    |
  +=========+
  |  edi    |
  +=========+
  |  ds     |
  +=========+   <- esp

*/
