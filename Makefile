CC=clang
CFLAGS=-Wpadded -std=c99 -m32 -c -Wall -ffreestanding -fno-stack-protector -Iincludes -Imultiboot -g 
LD=ld
LDFLAGS=-m elf_i386 -L bin -T linker.ld -static 	
	
AS=nasm
ASFLAGS=-felf32 -Fdwarf  
LODEV=/dev/loop0
OBJDIR:=bin
OBJS:=$(addprefix $(OBJDIR)/, multiboot.so cursor.so atoi.so atou.so itoa.so utoa.so utox.so strlen.so print.o kernel.o) 
VPATH=kernel:kernel/stdio:nasm:tests/stdio		# make searchdirs variable...

# settings floppy
GRUBFILE=setup_grub.txt
OUTPUT=/media/sf_VBoxLinuxShare/binaries/floppy.img	# TODO path remove etc...
IMG=floppy.img
MOUNTPOINT=/mnt/floppy

#settings HDD
GRUBFILEHD=setup_grub_hd.txt
OUTPUTHD=/media/sf_VBoxLinuxShare/binaries/hdd.img
IMGHD=hdd.img
LODEVHD=/dev/mapper/loop0p1

vpath %.h includes					# search for specific filetypes in <dir>

all: kernel.bin

$(OBJS): | $(OBJDIR)

# had to make this rule match *.so (shared object) 
# when referencing assembly files
$(OBJDIR)/%.so: %.asm
	$(AS) $(ASFLAGS) $< -o $@
# this matches all c-files
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@


$(OBJDIR):
	-mkdir $(OBJDIR) 
#$(OBJDIR)/multiboot.so: mymultiboot.asm
#	$(AS) $(ASFLAGS) $^ -o $@  
#atoi.o: atoi.asm
#	$(AS) $(ASFLAGS) $^ -o $@  
#atou.o: atou.asm
#	$(AS) $(ASFLAGS) $^ -o $@  
#strlen.o: strlen.asm
#	$(AS) $(ASFLAGS) $^ -o $@  
#itoa.o: itoa.asm
#	$(AS) $(ASFLAGS) $^ -o $@  
#utoa.o: utoa.asm
#	$(AS) $(ASFLAGS) $^ -o $@
#utox.o: utox.asm
#	$(AS) $(ASFLAGS) $^ -o $@
#cursor.o: cursor.asm
#	$(AS) $(ASFLAGS) $^ -o $@
kernel.bin: $(OBJS)  
	$(LD) $(LDFLAGS) $^ -o kernel.bin
	-mbchk $@
clean:
	-rm -f -r $(OBJS) kernel.o
	-rm -f $(IMG)	
	-rm -f kernel.bin
	-rm -f test 
	-rm -f tags
	-rm -f $(IMGHD) 
	-rm -rf $(OBJDIR)
$(IMGHD):	 
	dd if=/dev/zero of=$(IMGHD) bs=1024 count=10000			# makes a hdd image of size 10MB
	parted $(IMGHD) mklabel msdos mkpart primary 1 10 set 1 boot on # check for 1 10 in start, end for mkpart !
grubhd:	$(IMGHD) kernel.bin $(GRUBFILEHD) 
	kpartx -a $(IMGHD)						# assign partition(s) to loopback-device (dev/mapper/loop0p1)
	mkfs.ext2 -v $(LODEVHD)						# make filesystem on partition
	mount $(LODEVHD) $(MOUNTPOINT)					# mount partition 
	mkdir -p $(MOUNTPOINT)/boot/grub				# make grub dirs and copy files
	cp /usr/share/grub/i386-redhat/stage[12] $(MOUNTPOINT)/boot/grub
	cp kernel.bin $(MOUNTPOINT)/
	cp grub.conf $(MOUNTPOINT)/boot/grub
	grub --device-map=/dev/null --batch < $(GRUBFILEHD) 		# setup grub for HDD
	umount $(MOUNTPOINT)						# umount partition 
	kpartx -d $(IMGHD)						# release loopback-device
installhd: grubhd
	cp $(IMGHD) $(OUTPUTHD)						# copy to destination - remember to use VBoxManage convertfromraw command on image file 
									# if using VirtualBox.
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
tests:	$(OBJS) itoa.c 
	$(CC) -g -I. -o test $(filter-out $(OBJDIR)/multiboot.so $(OBJDIR)/kernel.o, $^) 
