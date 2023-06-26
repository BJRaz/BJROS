#settings HDD
#GRUBFILEHD=setup_grub_hd.txt

JAIL=jail
MOUNTPOINT=$(JAIL)/mnt
MOUNTPOINTFD=$(JAIL)/mnt/floppy
IMGHD=hdd.img
IMGHDDGPT=hddgpt.img
LODEVHD=/dev/mapper/loop0p1
LODEV=/dev/loop0
GRUBFLAGS=--target=i386-pc --locales=en@piglatin --fonts=ascii --install-modules="multiboot normal part_msdos ext2" --modules="multiboot normal part_msdos ext2" 
BUILDDIR=../build/x86

all:	grubhd 

$(JAIL):	
	./makejail.sh

$(IMGHD):	 
	dd if=/dev/zero of=$(IMGHD) bs=1024 count=30000				# makes a hdd image of size 30MB
	parted $(IMGHD) mklabel msdos mkpart primary 1 30 set 1 boot on 	# 
$(IMGHDDGPT):
	dd if=/dev/zero of=$(IMGHDDGPT) bs=512 count=1000000			# makes a hdd image of size 512MB
	parted $(IMGHDDGPT) mklabel gpt mkpart EFI 1 200 set 1 boot on 		#
grubhd:	$(IMGHD) $(JAIL) 
						# assign partition(s) to loopback-device (dev/mapper/loop0p1)
	kpartx -l $(IMGHD) 2>&1 
	kpartx -av $(IMGHD)
	mkfs.ext2 -v $(LODEVHD)						# make filesystem on partition
	mount $(LODEVHD) $(MOUNTPOINT)					# mount partition 
	mkdir -p $(MOUNTPOINT)/boot					# make grub dirs and copy files
	-cp $(BUILDDIR)/kernel.elf $(MOUNTPOINT)/
	mount --bind /proc $(JAIL)/proc
	mount --bind /dev $(JAIL)/dev
	-chroot $(JAIL) /grub2.sh
	-cp -p grub.cfg $(MOUNTPOINT)/boot/grub2/
	umount $(MOUNTPOINT)						# umount partition 
	umount $(JAIL)/proc
	umount $(JAIL)/dev
	-kpartx -dv $(IMGHD)						# release loopback-device
	sync
installhd: grubhd 
	cp $(IMGHD) $(OUTPUTHD)	
floppy:	kernel.elf $(IMG)
	#losetup $(LODEV) $(IMG)
	$(eval LODEV=$(shell losetup --show -f $(IMG)))
	mkfs.ext2 -v $(LODEV) 
	mount $(LODEV) $(MOUNTPOINTFD)
	mkdir $(MOUNTPOINTFD)/boot
	cp $(BUILDDIR)/kernel.elf $(MOUNTPOINTFD)
	grub2-install --target=i386-pc $(GRUBFLAGS) --force --allow-floppy --boot-directory=$(MOUNTPOINTFD)/boot $(LODEV)
	cp grub2/grub_floppy.cfg $(MOUNTPOINTFD)/boot/grub2/grub.cfg
	umount $(MOUNTPOINTFD)
	losetup -d $(LODEV)
installfloppy: floppy
	cp $(IMG) $(OUTPUT)
testeval:
	losetup -d $(LODEV)
clean:
	-rm -rf $(IMGHD) $(IMGHDGPT)	
	-rm -rf $(JAIL)
