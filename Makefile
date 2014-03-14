SYSTEM := $(shell uname -s)

ifeq ($(SYSTEM), Darwin)
PREFIX=$(ANDROID_NDK)/toolchains/x86-4.8/prebuilt/darwin-x86_64/bin/i686-linux-android-
else
PREFIX=
endif

CC_ASM            = $(PREFIX)gcc 
LD_ASM            = $(PREFIX)ld
OBJCOPY           = $(PREFIX)objcopy
CFLAGS            = -c -Wall -Werror
TRIM_FLAGS        = -R .pdr -R .comment -R .note -S -O binary
REDIRECT          = > /dev/null 2>&1
PIC               =

clean:
	rm -rf bin/*.o
	rm -rf bin/*.elf
	rm -rf bin/*.bin
	rm -rf iso/*.img
	rm -rf iso/mnt

compile:
	$(CC_ASM) $(CFLAGS) $(PIC) boot/boot.S -o bin/boot.o
	$(CC_ASM) $(CFLAGS) $(PIC) kernel/kernel.S -o bin/kernel.o
	$(CC_ASM) $(CFLAGS) $(PIC) kernel/interrupt.S -o bin/interrupt.o
	$(LD_ASM) bin/boot.o -o bin/boot.elf -Tboot/boot.ld
	$(OBJCOPY) $(TRIM_FLAGS) bin/boot.elf bin/boot.bin
	$(CC_ASM) $(PIC) -std=c99 -c -g -Os -march=i686 -ffreestanding -Wall -Werror kernel/main.c -o bin/main.o
	$(CC_ASM) $(PIC) -std=c99 -c -g -Os -march=i686 -ffreestanding -Wall -Werror kernel/vsnprintf.c -o bin/vsnprintf.o
	$(CC_ASM) $(PIC) -std=c99 -c -g -Os -march=i686 -ffreestanding -Wall -Werror kernel/io.c -o bin/io.o
	$(LD_ASM) -static -T kernel/kernel.ld -nostdlib --nmagic -o bin/kernel.elf bin/kernel.o bin/interrupt.o bin/main.o bin/vsnprintf.o bin/io.o
	$(OBJCOPY) -O binary bin/kernel.elf bin/kernel.bin
	dd if=bin/boot.bin of=iso/boot.img bs=512 count=1 $(REDIRECT)
	dd if=/dev/zero of=iso/boot.img skip=1 seek=1 bs=512 count=2879 $(REDIRECT)

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
