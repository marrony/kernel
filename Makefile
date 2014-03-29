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
PIC         = -fno-pic -fno-pie
KCFLAGS     = $(PIC) -I./include -std=c99 -c -g -Os -march=i686 -ffreestanding -Wall -Werror 

BOOT_OBJS = bin/boot.o

KERNEL_OBJS = \
	bin/start.o \
	bin/trap.o \
	bin/timer.o \
	bin/interrupt.o \
	bin/paging.o \
	bin/heap.o \
	bin/task.o \
	bin/syscall.o \
	bin/main.o \
	bin/kprintf.o \
	bin/descriptor.o \
	bin/string.o

clean:
	rm -rf bin/*.o
	rm -rf bin/*.elf
	rm -rf bin/*.bin
	rm -rf iso/*.img
	rm -rf iso/mnt

compile: $(BOOT_OBJS) $(KERNEL_OBJS)
	$(LD) bin/boot.o -o bin/boot.elf -Tboot/boot.ld
	$(OBJCOPY) $(TRIM_FLAGS) bin/boot.elf bin/boot.bin
	dd if=bin/boot.bin of=iso/boot.img bs=512 count=1 $(REDIRECT)
	dd if=/dev/zero of=iso/boot.img skip=1 seek=1 bs=512 count=2879 $(REDIRECT)
	$(LD) -static -T kernel/kernel.ld -nostdlib --nmagic -o bin/kernel.elf $(KERNEL_OBJS)
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

bin/%.o: kernel/%.c kernel/*.h include/*.h
	$(CC) $(KCFLAGS) $< -o $@
	
bin/%.o: kernel/%.S kernel/*.h include/*.h
	$(AS) $(CFLAGS) $(PIC) $< -o $@

bin/%.o: boot/%.S
	$(AS) $(CFLAGS) $< -o $@

