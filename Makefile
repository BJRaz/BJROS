CC=gcc
CFLAGS=-std=c99 -m32 -c -Wall -fno-builtin -fno-stack-protector -Iincludes 
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

vpath %.h includes

all:	mkdir kernel.bin
	-mbchk kernel.bin
mkdir:
	-mkdir bin
#print.o: print.c 
#	$(CC) $(CFLAGS) $^ -o $@ 
#kernel.o: kernel.c  
#	$(CC) $(CFLAGS) $^ -o $@

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
# -l:itoa.o -l:multiboot.o -l:kernel.o -l:print.o -l:atoi.o -l:strlen.o 
clean:
	-rm -f -r $(OBJS) kernel.o
	-rm -f $(IMG)	
	-rm -f kernel.bin
	-rm -f test
tests:	itoa.c
	$(CC) $(CFLAGS) -g -o test $^ 
$(IMG):	kernel.bin $(GRUBFILE) 
	dd if=/dev/zero of=$(IMG) bs=1024 count=1440
	losetup $(LODEV) $(IMG)
	mkfs $(LODEV)
	mount $(LODEV) $(MOUNTPOINT)
	mkdir -p $(MOUNTPOINT)/boot/grub
	cp /usr/share/grub/i386-redhat/stage[12] $(MOUNTPOINT)/boot/grub
	cp kernel.bin $(MOUNTPOINT)/
	cp grub.conf $(MOUNTPOINT)/boot/grub
	grub --device-map=/dev/null --batch < $(GRUBFILE) 
	umount $(MOUNTPOINT)
	losetup -d $(LODEV)
install: $(IMG) 
	cp $(IMG) $(OUTPUT)
brian: 	
	echo $^
