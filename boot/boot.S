#define BOOT_LOADER_CODE_AREA_ADDRESS         0x7c00
#define BOOT_LOADER_CODE_AREA_ADDRESS_OFFSET  0x0000

#define BOOT_LOADER_STACK_SEGMENT             0x7c00

#define BOOT_LOADER_ROOT_OFFSET               0x0200 // 512 after boot
#define BOOT_LOADER_FAT_OFFSET                0x0200

#define BOOT_LOADER_STAGE2_ADDRESS            0xffff //  0x1000
#define BOOT_LOADER_STAGE2_OFFSET             0x0010 //  0x0000 

#define BOOT_DISK_SECTORS_PER_TRACK           0x0012
#define BOOT_DISK_HEADS_PER_CYLINDER          0x0002
#define BOOT_DISK_BYTES_PER_SECTOR            0x0200
#define BOOT_DISK_SECTORS_PER_CLUSTER         0x0001

#define FAT12_FAT_POSITION                    0x0001
#define FAT12_FAT_SIZE                        0x0009
#define FAT12_ROOT_POSITION                   0x0013
#define FAT12_ROOT_SIZE                       0x000e
#define FAT12_ROOT_ENTRIES                    0x00e0
#define FAT12_END_OF_FILE                     0x0ff8

#define BOOT_SIGNATURE                        0xaa55

.macro writeString message
     pushw \message
     call  _writeString
     addw  $0x02, %sp
.endm

.macro readSector sectorno, address, offset, totalsectors
     pushw \sectorno
     pushw \address
     pushw \offset
     pushw \totalsectors
     call  _readSector
     addw  $0x08, %sp
.endm

.macro clusterToSectorNumber cluster
     pushw \cluster
     call  _clusterToSectorNumber
     addw  $0x02, %sp
.endm

.macro loadFile file
     /* read root directory into memory */
     readSector $FAT12_ROOT_POSITION, $BOOT_LOADER_CODE_AREA_ADDRESS, $BOOT_LOADER_ROOT_OFFSET, $FAT12_ROOT_SIZE
     pushw \file
     call  _findFile
     addw  $0x02, %sp

     pushw %ax
     /* read fat table into memory */
     readSector $FAT12_FAT_POSITION, $BOOT_LOADER_CODE_AREA_ADDRESS, $BOOT_LOADER_FAT_OFFSET, $FAT12_FAT_SIZE

     popw  %ax
     movw  $BOOT_LOADER_STAGE2_OFFSET, %bx
_loadCluster:
     pushw %bx
     pushw %ax

     clusterToSectorNumber %ax
     readSector %ax, $BOOT_LOADER_STAGE2_ADDRESS, %bx, $BOOT_DISK_SECTORS_PER_CLUSTER

     popw  %ax
     xorw %dx, %dx
     movw $0x03, %bx
     mulw %bx
     movw $0x02, %bx
     divw %bx

     movw $BOOT_LOADER_FAT_OFFSET, %bx
     addw %ax, %bx
     movw $BOOT_LOADER_CODE_AREA_ADDRESS, %ax
     movw %ax, %es
     movw %es:(%bx), %ax
     orw  %dx, %dx
     jz   _even_cluster
_odd_cluster:
     shrw $0x04, %ax
     jmp  _done 
_even_cluster:
     and $0x0fff, %ax
_done:
     popw %bx
     addw $BOOT_DISK_BYTES_PER_SECTOR, %bx
     cmpw $FAT12_END_OF_FILE, %ax
     jl _loadCluster
.endm

.macro setupSegments
    cli
    movw  %cs, %ax
    movw  %ax, %ds
    movw  %ax, %es
    movw  %ax, %ss
    movw  $BOOT_LOADER_STACK_SEGMENT, %sp
    sti
.endm

.macro fastA20
    inb $0x92, %al
    test $2, %al
    jnz _fastA20Exit
    orb $2, %al
    andb $0xfe, %al
    outb %al, $0x92
_fastA20Exit:
.endm

.macro initKernel
     /* initialize the kernel */
init_gdt:
    lgdt gdt_ptr
    
    mov $0x01, %ax
    lmsw %ax          # set fist cr0 bit

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    mov $0xffff, %ax
    mov %ax, %sp
    mov %ax, %bp

    .byte 0x66
    .byte 0xea
    .long 0x100000
    .word 0x08
    #ljmp $0x08, $0x100000
.endm

.macro GDT_ENTRY base limit access granularity
    .word \limit & 0xffff
    .word \base & 0xffff
    .byte (\base >> 16) & 0xff
    .byte \access
    .byte (\granularity & 0xf0) | ((\limit >> 16) & 0x0f)
    .byte (\base >> 24) & 0xff
.endm

//////////////////////////////////////////////////////////////////////////////////

.code16
.text
.globl _start

_start:
     jmp _boot
     nop

/* description of each entity        : value          type                    */
/* oem label                         : MARRONY0     , type: string, length: 8 */

/*                          BIOS Parameter Block                              */
/* total bytes per sector            : 512          , type: word              */
/* total sectors per cluster         : 1            , type: byte              */
/* total reserved sectors            : 1            , type: word              */
/* total fat tables                  : 2            , type: byte              */
/* total directory entries           : 224          , type: word              */
/* total sectors                     : 2880         , type: word              */
/* media description                 : 0xf0         , type: byte              */
/* size in sectors of each fat table : 9            , type: word              */
/* total sectors per track           : 18           , type: word              */
/* total heads per cylinder          : 2            , type: word              */
/* total hidden sectors              : 0            , type: double word       */
/* total big sectors                 : 0            , type: double word       */

/*                      Extended BIOS Parameter Block                         */
/* boot drive identifier             : 0            , type: byte              */
/* total unused sectors              : 0            , type: byte              */
/* external boot signature           : 0x29         , type: byte              */
/* serial number                     : 0xffffffff   , type: string, length: 4 */
/* volume label first 6 bytes of 11  : MARRON       , type: string, length: 6 */
/* volume label second 5 bytes of 11 : Y NER        , type: string, length: 5 */
/* file system type                  : FAT12        , type: string, length: 8 */

     .ascii "MARRONY "
     .byte 0x00,0x02
     .byte 0x01
     .byte 0x01,0x00
     .byte 0x02
     .byte 0xe0,0x00
     .byte 0x40,0x0b
     .byte 0xf0
     .byte 0x09,0x00
     .byte 0x02,0x01
     .byte 0x02,0x00
     .byte 0x00,0x00, 0x00, 0x00
     .byte 0x00,0x00, 0x00, 0x00
     .byte 0x00
     .byte 0x00
     .byte 0x29
     .byte 0xff, 0xff, 0xff, 0xff
     .ascii "MARRON"
     .ascii "Y 1.0"
     .ascii "FAT12   "

_boot:
    setupSegments
    fastA20
    loadFile $kernel_bin
    initKernel

_freeze:
    jmp _freeze

_abort:
    writeString $msgAbort
    hlt
    jmp _freeze

/* void writeString(const char* str); */
_writeString:
    pushw %bp
    movw  %sp, %bp
    movw 4(%bp), %si
    jmp  _writeStringCheckByte
_writeStringIn:
    movb $0x0e, %ah
    movb $0x00, %bh
    int  $0x10
    incw %si
_writeStringCheckByte:
    movb (%si), %al
    orb  %al, %al
    jnz  _writeStringIn
_writeStringOut:
    movw %bp, %sp
    popw %bp
    ret

/* void readSector(short sector, void* address, short offset, short sectors); */
/*
 * 4(%bp) = sectors
 * 6(%bp) = offset
 * 8(%bp) = address
 * 10(%bp) = sector
 */
_readSector:
    pushw %bp
    movw %sp, %bp
    movw 10(%bp), %ax

    // ax = sector / BOOT_DISK_PER_TRACK
    // cl = (sector % BOOT_DISK_PER_TRACK) + 1
    movw $BOOT_DISK_SECTORS_PER_TRACK, %bx
    xorw %dx, %dx
    divw %bx

    incw %dx
    movb %dl, %cl

    // ch = ax / BOOT_DISK_HEADS_PER_CYLINDER
    // dx = ax % BOOT_DISK_HEADS_PER_CYLINDER
    movw $BOOT_DISK_HEADS_PER_CYLINDER, %bx
    xorw %dx, %dx
    divw %bx

    movb %al, %ch
    xchg %dl, %dh

    movb $0x02, %ah
    movb 4(%bp), %al
    movb bootDrive, %dl
    movw 8(%bp), %bx
    movw %bx, %es
    movw 6(%bp), %bx
    int  $0x13
    jc _abort
    cmpb 4(%bp), %al
    jc _abort

    movw %bp, %sp
    popw %bp
    ret

/* short findFile(const char* filename); */
_findFile:
    pushw %bp
    movw  %sp, %bp

    movw  $BOOT_LOADER_CODE_AREA_ADDRESS, %ax
    movw  %ax, %es
    movw  $BOOT_LOADER_ROOT_OFFSET, %bx
    movw  $FAT12_ROOT_ENTRIES, %dx

_findFileLoop:
    cmpw $0x00, %dx
    je _abort
    movw  $0x0b, %cx
    movw  4(%bp), %si
    leaw  (%bx), %di
    repe  cmpsb
    je _findFileExit
    decw  %dx
    addw  $0x20, %bx
    jmp _findFileLoop

_findFileExit:
    addw  $0x1a, %bx
    movw  %es:(%bx), %ax
    movw  %bp, %sp
    popw  %bp
    ret

/* short clusterToSectorNumber(short cluster); */
_clusterToSectorNumber:
    pushw %bp
    movw  %sp, %bp
    movw  4(%bp) , %ax
    subw  $0x02, %ax
    movw  $BOOT_DISK_SECTORS_PER_CLUSTER, %cx
    mulw  %cx
    addw  $FAT12_ROOT_POSITION, %ax
    addw  $FAT12_ROOT_SIZE, %ax
    movw  %bp, %sp
    popw  %bp
    ret

    bootDrive : .byte 0x0000
    msgAbort  : .asciz "F A T A L  E R R O R"
    kernel_bin: .ascii  "KERNEL  BIN"

gdt_begin:
    GDT_ENTRY 0 0x00000000 0x00 0x00 # null segment
    GDT_ENTRY 0 0xffffffff 0x9b 0xc0 # code segment
    GDT_ENTRY 0 0xffffffff 0x93 0xc0 # data segment
gdt_end:

gdt_ptr:
    .word (gdt_end - gdt_begin) - 1
    .long gdt_begin 

    . = _start + 0x01fe

    .word BOOT_SIGNATURE

