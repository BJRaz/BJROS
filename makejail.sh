#!/bin/sh

# JAIL SETUP
mkdir -p jail/bin jail/lib64 jail/usr/sbin jail/usr/share/locale
mkdir -p jail/usr/lib
mkdir -p jail/proc jail/dev
mkdir -p jail/mnt
cp -R /usr/lib/grub jail/usr/lib

#/bin/ls lib dependencies 
cp /lib64/libdl.so.2 /lib64/libc.so.6 /lib64/ld-linux-x86-64.so.2 /lib64/libtinfo.so.5 /lib64/libselinux.so.1 /lib64/libcap.so.2 /lib64/libacl.so.1 /lib64/libpcre.so.1 /lib64/libattr.so.1 /lib64/libpthread.so.0 jail/lib64/
#/usr/sbin/grub2-install dependencies
cp /lib64/liblzma.so.5 /lib64/libdevmapper.so.1.02 /lib64/libsepol.so.1 /lib64/libudev.so.1 /lib64/libm.so.6 /lib64/librt.so.1 /lib64/libcap.so.2 /lib64/libdw.so.1 /lib64/libgcc_s.so.1 /lib64/libelf.so.1 /lib64/libz.so.1 /lib64/libbz2.so.1 jail/lib64/

cp /bin/bash jail/bin
cp /bin/ls jail/bin
cp -R /usr/share/locale/en@piglatin jail/usr/share/locale
cp grub2/grub2.sh jail/
cp -R /usr/sbin/grub2* jail/usr/sbin

