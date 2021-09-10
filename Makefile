CC=clang
#CFLAGS=-nostdinc -Wpadded -std=c99 -m32 -c -Wall -fno-stack-protector -Iinclude -Imultiboot 
CFLAGS=-nostdinc -nobuiltininc -Wpadded -std=c99 -m32 -c -Wall -ffreestanding -fno-stack-protector -Iinclude -Imultiboot 
LD=ld
LDFLAGS=-m elf_i386 -L bin -T linker.ld -static 
#LDFLAGS=-m elf_i386 -T linker.ld -lstdc++ -L /usr/lib/gcc/i686-redhat-linux/10 --static #/usr/lib/crt1.o 
#-M 
AS=nasm
ASFLAGS=-felf32 -Fdwarf   
LODEV=/dev/loop0
OBJDIR:=bin
OBJS:=$(addprefix $(OBJDIR)/, multiboot.so cursor.so atoi.so atou.so itoa.so utoa.so utox.so strlen.so print.o string.so kernel.o) 
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

vpath %.h include					# search for specific filetypes in <dir>

all: kernel.elf TAGS

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
kernel.elf: $(OBJS)  
	$(LD) $(LDFLAGS) $^ -o kernel.elf
	-mbchk $@
clean:
	-rm -f -r $(OBJS) kernel.o
	-rm -f $(IMG)	
	-rm -f kernel.elf
	-rm -f test 
	-rm -f tags
	-rm -f $(IMGHD) 
	-rm -rf $(OBJDIR)
$(IMGHD):	 
	dd if=/dev/zero of=$(IMGHD) bs=1024 count=30000			# makes a hdd image of size 30MB
	parted $(IMGHD) mklabel msdos mkpart primary 1 30 set 1 boot on # check for 1 10 in start, end for mkpart !
grubhd:	$(IMGHD) kernel.elf $(GRUBFILEHD) 
	kpartx -a $(IMGHD)						# assign partition(s) to loopback-device (dev/mapper/loop0p1)
	mkfs.ext2 -v $(LODEVHD)						# make filesystem on partition
	mount $(LODEVHD) $(MOUNTPOINT)					# mount partition 
	mkdir -p $(MOUNTPOINT)/boot/grub				# make grub dirs and copy files
	cp ./grub-0.97-i386-pc/boot/grub/stage[12] $(MOUNTPOINT)/boot/grub
	#cp vmlinuz $(MOUNTPOINT)/
	#cp config $(MOUNTPOINT)/
	#cp initramfs $(MOUNTPOINT)/
	cp kernel.elf $(MOUNTPOINT)/
	cp grub.conf $(MOUNTPOINT)/boot/grub
	grub --device-map=/dev/null --batch < $(GRUBFILEHD) 		# setup grub for HDD
	umount $(MOUNTPOINT)						# umount partition 
	kpartx -d $(IMGHD)						# release loopback-device
installhd: grubhd
	cp $(IMGHD) $(OUTPUTHD)						# copy to destination - remember to use VBoxManage convertfromraw command on image file 
									# if using VirtualBox.
$(IMG):	 
	dd if=/dev/zero of=$(IMG) bs=1024 count=1440
grub:	$(IMG) kernel.elf $(GRUBFILE) 
	losetup $(LODEV) $(IMG)
	mkfs.vfat -F 16 -v $(LODEV)
	mount $(LODEV) $(MOUNTPOINT)
	mkdir -p $(MOUNTPOINT)/boot/grub
	#cp /usr/share/grub/i386-redhat/stage[12] $(MOUNTPOINT)/boot/grub
	cp ./grub-0.97-i386-pc/boot/grub/stage[12] $(MOUNTPOINT)/boot/grub
	cp kernel.elf $(MOUNTPOINT)/
	cp assets/moon-scene_big.xbm.gz $(MOUNTPOINT)/
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
	ctags --exclude=k.c -R .
tests:	$(OBJS) itoa.c 
	#$(CC) -g -I. -o test $(filter-out $(OBJDIR)/multiboot.so $(OBJDIR)/kernel.o, $^) 
	$(CC) -m32 -I. -g tests/stdio/itoa.c -o test bin/atoi.so bin/atou.so bin/utoa.so bin/itoa.so bin/utox.so bin/strlen.so bin/string.so 
