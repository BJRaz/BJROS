#!/bin/bash
#/usr/sbin/grub2-install --target=i386-pc --locales=en@piglatin --fonts=ascii --efi-directory=/mnt/boot /dev/loop0 
/usr/sbin/grub2-install --target=i386-pc --locales=en@piglatin --fonts=ascii --install-modules="multiboot normal part_msdos ext2" --modules="multiboot normal part_msdos ext2" --boot-directory=/mnt/boot /dev/loop0 
#/usr/sbin/grub2-install --efi-directory=/mnt/boot/efi --boot-directory=/mnt/boot/efi /dev/loop0

