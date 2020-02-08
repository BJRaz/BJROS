CC=gcc
CFLAGS=-m32 -c -Wall -fno-builtin -fno-stack-protector -Iincludes  
LD=ld
LDFLAGS=-L bin -T linker.ld -static	
	
AS=nasm
ASFLAGS=-felf32
LODEV=/dev/loop0

All:	mkdir kernel.bin
	mbchk kernel.bin
mkdir:
	-mkdir bin
bin/print.o:
	$(CC) $(CFLAGS) -o bin/print.o kernel/stdio/print.c
bin/kernel.o:	bin/print.o
	$(CC) $(CFLAGS) -o bin/kernel.o kernel/kernel.c
asm:
	$(AS) $(ASFLAGS) -o bin/multiboot.o nasm/mymultiboot.asm
	$(AS) $(ASFLAGS) -o bin/atoi.o kernel/stdio/atoi.asm
	$(AS) $(ASFLAGS) -o bin/strlen.o kernel/stdio/strlen.asm
kernel.bin:	asm bin/kernel.o 
	$(LD) $(LDFLAGS) -o kernel.bin -l:multiboot.o -l:kernel.o -l:print.o -l:atoi.o -l:strlen.o 
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

