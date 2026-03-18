# This makefile initiates a jail (chroot) for further procession of grub2
# to prevent curruption of the /boot system on the current system
# An hdd image file is created and partitioned
# The partition is initialized and set for boot
# The image file is mounted and grub2-install is run in grub2.sh after prerequisites folders are binded
JAIL=jail
MOUNTPOINT=$(JAIL)/mnt
MOUNTPOINTFD=$(JAIL)/mnt/floppy
IMGHD=hdd.img
LODEVHD=/dev/mapper/loop0p1
LODEV=/dev/loop0
BUILDDIR=../build/x86

all:	grubhd 

$(JAIL):	
	./makejail.sh

$(IMGHD):	 
	dd if=/dev/zero of=$(IMGHD) bs=1024 count=30000				# makes a hdd image of size 30MB
	parted $(IMGHD) mklabel msdos mkpart primary 1 30 set 1 boot on 	# partitions the image with partitionlabel msdos, with one partition 30MB and boot flag set 
grubhd:	$(IMGHD) $(JAIL) 
	# assign partition(s) to loopback-device (dev/mapper/loop0p1)
	$(eval LODEVHD=/dev/mapper/$(shell kpartx -av $(IMGHD) | grep -o 'loop[0-9]p1'))
	$(eval LODEV=/dev/$(shell echo $(LODEVHD) | grep -o 'loop[0-9]'))
	#kpartx -l $(IMGHD) 2>&1 
	#kpartx -av $(IMGHD)
	mkfs.ext2 $(LODEVHD)						# make filesystem on partition
	mount $(LODEVHD) $(MOUNTPOINT)					# mount partition 
	mkdir -p $(MOUNTPOINT)/boot					# make grub dirs and copy files
	-cp $(BUILDDIR)/kernel.elf $(MOUNTPOINT)/
	mount --bind /proc $(JAIL)/proc
	mount --bind /dev $(JAIL)/dev
	-chroot $(JAIL) /grub2.sh $(LODEV)
	-cp -p grub.cfg $(MOUNTPOINT)/boot/grub2/
	umount $(MOUNTPOINT)						# umount partition 
	umount $(JAIL)/proc
	umount $(JAIL)/dev
	-kpartx -d $(IMGHD)						# release loopback-device
	sync
installhd: grubhd 
	cp $(IMGHD) $(OUTPUTHD)	
clean:
	-rm -rf $(IMGHD) $(IMGHDGPT)	
	-rm -rf $(JAIL)
