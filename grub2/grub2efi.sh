#!/bin/bash
if [ -z $1 ];then
	echo "Missing device argument 1"
	exit 1
fi

/usr/sbin/grub2-install 					\
	--removable 						\
	--force 						\
	--target=x86_64-efi 					\
	--efi-directory=/mnt 					\
	--boot-directory=/mnt 					\
	--bootloader-id=grub 					\
	$1							#/dev/loop0

