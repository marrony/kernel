PREFIX=$ANDROID_NDK/toolchains/x86-4.8/prebuilt/darwin-x86_64/bin/i686-linux-android-

${PREFIX}gcc -c -Wall -Werror fs.S -o fs.o
${PREFIX}ld fs.o -o fs.elf
${PREFIX}objcopy -R .pdr -R .comment -R .note -S -O binary fs.elf fs.raw
rm -rf disk.vhd
VBoxManage convertfromraw fs.raw disk.vhd --format VHD

#fdisk -i -y -a dos -S 258 fs.raw 
#rm -rf disk.vhd
#VBoxManage convertfromraw fs.raw disk.vhd --format VHD

#dd if=/dev/zero of=disk.img count=8192
#dd if=setjmp.c of=disk.img count=1 oseek=8191
#fdisk -i -y -a dos disk.img 

