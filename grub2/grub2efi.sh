#!/bin/bash
/usr/sbin/grub2-install --removable --force --target=x86_64-efi --efi-directory=/mnt --boot-directory=/mnt --bootloader-id=grub /dev/loop0

