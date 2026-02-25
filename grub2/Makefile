
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
	dd if=/dev/zero of=$(IMGHD) bs=1024 count=80000				# makes a hdd image of size 30MB
	parted $(IMGHD) mklabel msdos mkpart primary 1 80 set 1 boot on 	# partitions the image with partitionlabel msdos, with one partition 30MB and boot flag set 
grubhd:	$(IMGHD) $(JAIL) 
	# assign partition(s) to loopback-device (dev/mapper/loop0p1)
	kpartx -l $(IMGHD) #2>&1 
	kpartx -a $(IMGHD)
	mkfs.ext2 $(LODEVHD)						# make filesystem on partition
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
	-kpartx -d $(IMGHD)						# release loopback-device
	sync
installhd: grubhd 
	cp $(IMGHD) $(OUTPUTHD)	
testeval:
	losetup -d $(LODEV)
clean:
	-rm -rf $(IMGHD) $(IMGHDGPT)	
	-rm -rf $(JAIL)
