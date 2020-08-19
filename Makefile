CC=gcc
CFLAGS=-Wpadded -std=c99 -m32 -c -Wall -ffreestanding -fno-stack-protector -Iincludes -g 
LD=ld
LDFLAGS=-m elf_i386 -L bin -T linker.ld -static	
	
AS=nasm
ASFLAGS=-felf32 -Fdwarf  
LODEV=/dev/loop0
OBJS=multiboot.o atoi.o itoa.o strlen.o print.o 
VPATH=kernel:kernel/stdio:nasm:tests/stdio		# make searchdirs variable...

GRUBFILE=setup_grub.txt
OUTPUT=/media/sf_VBoxLinuxShare/binaries/floppy.img
IMG=floppy.img
MOUNTPOINT=/mnt/floppy

vpath %.h includes					# search for specific filetypes in <dir>

all:	kernel.bin

mkdir:
	-mkdir bin
multiboot.o: mymultiboot.asm
	$(AS) $(ASFLAGS) $^ -o $@  
atoi.o: atoi.asm
	$(AS) $(ASFLAGS) $^ -o $@  
strlen.o: strlen.asm
	$(AS) $(ASFLAGS) $^ -o $@  
itoa.o: itoa.asm
	$(AS) $(ASFLAGS) $^ -o $@  
kernel.bin: $(OBJS) kernel.o 
	$(LD) $(LDFLAGS) $^ -o kernel.bin
	-mbchk $@
clean:
	-rm -f -r $(OBJS) kernel.o
	-rm -f $(IMG)	
	-rm -f kernel.bin
	-rm -f test
tests:	itoa.c atoi.o itoa.o print.o strlen.o 
	$(CC) -g -I. -o test $^ 
$(IMG):	 
	dd if=/dev/zero of=$(IMG) bs=1024 count=1440
grub:	$(IMG) kernel.bin $(GRUBFILE) 
	losetup $(LODEV) $(IMG)
	mkfs.vfat -F 16 -v $(LODEV)
	mount $(LODEV) $(MOUNTPOINT)
	mkdir -p $(MOUNTPOINT)/boot/grub
	cp /usr/share/grub/i386-redhat/stage[12] $(MOUNTPOINT)/boot/grub
	cp kernel.bin $(MOUNTPOINT)/
	cp grub.conf $(MOUNTPOINT)/boot/grub
	grub --device-map=/dev/null --batch < $(GRUBFILE) 
	umount $(MOUNTPOINT)
	losetup -d $(LODEV)
install: grub
	cp $(IMG) $(OUTPUT)
bochs: install
	bochs "boot:floppy" "floppya: 1_44=floppy.img, status=inserted"
qemu: install
	qemu -fda floppy.img 
TAGS:
	ctags -R .

