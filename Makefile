CC=gcc
LN=ld
AS=nasm
LODEV=/dev/loop0

All:	
	-mkdir bin
	$(CC) -m32 -c -Wall -fno-builtin -fno-stack-protector -o bin/print.o kernel/stdio/print.c
	$(CC) -m32 -c -Iincludes -Wall -fno-builtin -fno-stack-protector -o bin/kernel.o kernel/kernel.c
	$(AS) -f elf32  -o bin/multiboot.o nasm/mymultiboot.asm
	$(AS) -f elf32  -o bin/atoi.o kernel/stdio/atoi.asm
	$(AS) -f elf32  -o bin/strlen.o kernel/stdio/strlen.asm
	$(LN) -M -o kernel.bin -L bin -l:multiboot.o -l:kernel.o -l:print.o -l:atoi.o -l:strlen.o -T linker.ld -static	
	
	mbchk kernel.bin
clean:
	-rm -f -r bin
	-rm -f floppy.img	
	-rm -f kernel.bin

grub: 
	dd if=/dev/zero of=floppy.img bs=1024 count=1440
	losetup $(LODEV) floppy.img
	mkfs $(LODEV)
	mount $(LODEV) /mnt/floppy
	mkdir -p /mnt/floppy/boot/grub
	cp /usr/share/grub/i386-redhat/stage[12] /mnt/floppy/boot/grub
	cp kernel.bin /mnt/floppy
	cp grub.conf /mnt/floppy/boot/grub
	grub --device-map=/dev/null --batch < setup_grub.txt
	umount /mnt/floppy
	losetup -d $(LODEV)
install: grub
	cp floppy.img /media/sf_VBoxLinuxShare/binaries/floppy.img

