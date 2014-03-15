SYSTEM := $(shell uname -s)

ifeq ($(SYSTEM), Darwin)
PREFIX=$(ANDROID_NDK)/toolchains/x86-4.8/prebuilt/darwin-x86_64/bin/i686-linux-android-
else
PREFIX=
endif

AS          = $(PREFIX)gcc 
LD          = $(PREFIX)ld
CC          = $(PREFIX)gcc 
OBJCOPY     = $(PREFIX)objcopy
CFLAGS      = -c -Wall -Werror
TRIM_FLAGS  = -R .pdr -R .comment -R .note -S -O binary
REDIRECT    = > /dev/null 2>&1
PIC         = #-fno-pic -fno-pie # with these flags is causing int13
KCFLAGS     = $(PIC) -I./include -std=c99 -c -g -Os -march=i686 -ffreestanding -Wall -Werror 

clean:
	rm -rf bin/*.o
	rm -rf bin/*.elf
	rm -rf bin/*.bin
	rm -rf iso/*.img
	rm -rf iso/mnt

compile:
	$(AS) $(CFLAGS) boot/boot.S -o bin/boot.o
	$(LD) bin/boot.o -o bin/boot.elf -Tboot/boot.ld
	$(OBJCOPY) $(TRIM_FLAGS) bin/boot.elf bin/boot.bin
	dd if=bin/boot.bin of=iso/boot.img bs=512 count=1 $(REDIRECT)
	dd if=/dev/zero of=iso/boot.img skip=1 seek=1 bs=512 count=2879 $(REDIRECT)
	$(AS) $(CFLAGS) $(PIC) kernel/kernel.S -o bin/kernel.o
	$(AS) $(CFLAGS) $(PIC) kernel/interrupt.S -o bin/interrupt.o
	$(AS) $(CFLAGS) $(PIC) kernel/i386.S -o bin/i386.o
	$(CC) $(KCFLAGS) kernel/main.c -o bin/main.o
	$(CC) $(KCFLAGS) kernel/kprintf.c -o bin/kprintf.o
	$(CC) $(KCFLAGS) kernel/protect.c -o bin/protect.o
	$(CC) $(KCFLAGS) kernel/string.c -o bin/string.o
	$(LD) -static -T kernel/kernel.ld -nostdlib --nmagic -o bin/kernel.elf \
	bin/kernel.o \
	bin/interrupt.o \
	bin/i386.o \
	bin/main.o \
	bin/kprintf.o \
	bin/protect.o \
	bin/string.o
	$(OBJCOPY) -O binary bin/kernel.elf bin/kernel.bin

setup:
	mkdir -p iso/mnt
ifeq ($(SYSTEM), Darwin)
	hdid -mountpoint iso/mnt/ iso/boot.img
else
	sudo mount -o loop iso/boot.img iso/mnt/ -t fat12
endif
	cp bin/kernel.bin iso/mnt/
	sudo umount iso/mnt/
	rm -rf iso/mnt/

all:
	make clean
	make compile
	make setup
