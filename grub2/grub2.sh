#!/bin/bash
#/usr/sbin/grub2-install --target=i386-pc --locales=en@piglatin --fonts=ascii --efi-directory=/mnt/boot /dev/loop0 
#/usr/sbin/grub2-install --efi-directory=/mnt/boot/efi --boot-directory=/mnt/boot/efi /dev/loop0

if[$0 = ""];then
	echo "Missing device argument 0"
	exit 1
fi

/usr/sbin/grub2-install --target=i386-pc \
	--locales=en@piglatin \
	--fonts=ascii \
	--install-modules="multiboot normal part_msdos ext2" \
	--modules="multiboot normal part_msdos ext2" \
	--boot-directory=/mnt/boot \
	$0

exit 0

#	/dev/loop0 
