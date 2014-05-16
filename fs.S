#define BLOCK_SIZE 1024

#define BLOCK(x)           \
. = (x + 1) * BLOCK_SIZE ; \
block##x##:

. = 0
mbr:

. = 446
partition0:
    .byte 0x80
    .byte 0       #head
    .byte 0       #sector
    .byte 0       #cylinder
    .byte 0x80
    .byte 0       #head
    .byte 0       #sector
    .byte 0       #cylinder
    .long 2
    .long 8192 

. = 510
    .byte 0x55
    .byte 0xAA

BLOCK(0)
super_block:

free_list:
    .byte 1                         # block0 used (super block)
    .byte 1                         # block1 used (inode list)
    .byte 1                         # block2 used (root dir content)
    .byte 1                         # block3 used (file1 content)
    .byte 1                         # block4 used (file3 content)
    .byte 1                         # block5 used (dir content)
    .byte 0                         # another blocks are unused

BLOCK(1)
inode0:
    .byte  2                        # type  1 -> file 2 -> directory
    .long  1                        # file size
    .long  2, 0, 0, 0, 0, 0, 0, 0   # direct block
    .byte  1                        # used/free
inode1:
    .byte  1                        # type  1 -> file 2 -> directory
    .long  1                        # file size
    .long  3, 0, 0, 0, 0, 0, 0, 0   # direct block
    .byte  1                        # used/free
inode2:
    .byte  1                        # type  1 -> file 2 -> directory
    .long  1                        # file size
    .long  4, 0, 0, 0, 0, 0, 0, 0   # direct block
    .byte  1                        # used/free
inode3:
    .byte  2                        # type  1 -> file 2 -> directory
    .long  1                        # file size
    .long  5, 0, 0, 0, 0, 0, 0, 0   # direct block
    .byte  1                        # used/free
inode4:
    .byte  1                        # type  1 -> file 2 -> directory
    .long  1                        # file size
    .long  6, 0, 0, 0, 0, 0, 0, 0   # direct block
    .byte  1                        # used/free
inode5:
    .byte  1                        # type  1 -> file 2 -> directory
    .long  1                        # file size
    .long  7, 0, 0, 0, 0, 0, 0, 0   # direct block
    .byte  1                        # used/free

BLOCK(2)
    .ascii "file1\0\0\0"            # file name
    .long 1                         # ino file1
    .ascii "file2\0\0\0"            # file name
    .long 2                         # ino file2
    .ascii "dir\0\0\0\0\0"          # file name
    .long 3                         # ino dir

BLOCK(3)
    .ascii "this is the content of the file\n"
    .ascii "this file should be cool\n"

BLOCK(4)
    .ascii "this is another file\n"

BLOCK(5)
    .ascii "file3\0\0\0"            # file name
    .long 4                         # ino file3
    .ascii "file4\0\0\0"            # file name
    .long 5                         # ino file4

BLOCK(6)
    .ascii "content of file3"

BLOCK(7)
    .ascii "content of file4"

BLOCK(128)

BLOCK(129)

