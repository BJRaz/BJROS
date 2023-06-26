CC=gcc
LN=ld
LNARGS=
AS=nasm
LODEV=/dev/loop0
MNT=mount/floppy

all:	
	-mkdir bin
	-mkdir -p mount/floppy 
	-mkdir build
	$(CC) -m32 -c -Wall -fno-builtin -fno-stack-protector -o bin/print.o kernel/stdio/print.c
	$(CC) -m32 -c -Iincludes -Wall -fno-builtin -fno-stack-protector -o bin/kernel.o kernel/kernel.c
	$(AS) -f elf32  -o bin/multiboot.o nasm/mymultiboot.asm
	$(AS) -f elf32  -o bin/atoi.o kernel/stdio/atoi.asm
	$(AS) -f elf32  -o bin/strlen.o kernel/stdio/strlen.asm
	$(LN) $(LNARGS) -m elf_i386 -M -o build/kernel.bin -L bin \
		-l:multiboot.o -l:kernel.o \
		-l:print.o -l:atoi.o -l:strlen.o \
		-T linker.ld -static	
	
	-mbchk build/kernel.bin
clean:
	-rm -f -r bin mount build
	-rm -f floppy.img	
	-rm -f build/kernel.bin
floppy.img:	all
	dd if=/dev/zero of=floppy.img bs=1024 count=1440
	losetup $(LODEV) floppy.img
grub:	floppy.img 
	mkfs $(LODEV)
	mount $(LODEV) $(MNT)
	mkdir -p $(MNT)/boot/grub
	cp grub-0.97-i386-pc/boot/grub/stage[12] $(MNT)/boot/grub
	cp build/kernel.bin $(MNT)
	cp grub.conf $(MNT)/boot/grub
	grub --device-map=/dev/null --batch < setup_grub.txt
	umount $(MNT)
	losetup -d $(LODEV)
install: grub
	cp floppy.img /media/sf_VBoxLinuxShare/binaries/floppy.img

