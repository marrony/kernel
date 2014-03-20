ENTRY(start)
SECTIONS
{
    . = 0x100000;

    __kernel_start = . ;

    .text :
    {
        __text_start = . ;
        *(.text);
        . = ALIGN(4096);
        __text_end = . ;
    }

    .data ALIGN(4096) :
    {
        __data_start = . ;
        *(.data);
        *(.rodata);
        . = ALIGN(4096);
        __data_end = . ;
    }

    .bss ALIGN(4096) :
    {
        __bss_start = . ;
        *(.bss);
        . = ALIGN(4096);
        __bss_end = . ;
    }

    __kernel_end = . ;
} 
