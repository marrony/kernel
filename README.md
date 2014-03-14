kernel
======

This is part of my studies on OSDev

For now I have a simple bootloader and a simple kernel.

The bootloader do:
- enables A20 line
- load the 'kernel.bin' file from FAT12 to memory 0x100000
- setup a simple GDT
- enters in protected mode
- jump to 0x100000

The kernel do:
- setup a stack
- setup a GDT with user/kernel segments
- setup a IDT with dummy interrupt handlers
- install a timer
- prints a hello message and clock ticks

