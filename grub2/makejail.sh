#!/bin/sh
check_cp() 
{
	if [ $? -eq 1 ];then
		echo " ** Copy failed ..."
		exit 1
	fi
}

check_mkdir()
{
	if [ $? -eq 1 ]; then
		echo " ** Makedir failed"
		exit 1
	fi	
}
# JAIL SETUP
for directory in jail/bin jail/lib64 jail/usr/sbin jail/usr/share/locale jail/usr/lib jail/proc jail/dev jail/mnt; do
	mkdir -p $directory
done

cp -R /usr/lib/grub jail/usr/lib

#/bin/ls lib dependencies 
for lib in /lib64/libdl.so.2 /lib64/libc.so.6 /lib64/ld-linux-x86-64.so.2 /lib64/libtinfo.so.6 /lib64/libselinux.so.1 /lib64/libcap.so.2 /lib64/libacl.so.1 /lib64/libpcre.so.1 /lib64/libattr.so.1 /lib64/libpthread.so.0;do
	cp $lib jail/lib64
	check_cp
done

#/usr/sbin/grub2-install dependencies
for lib in /lib64/libpcre2-8.so.0 /lib64/liblzma.so.5 /lib64/libdevmapper.so.1.02 /lib64/libsepol.so.2 /lib64/libudev.so.1 /lib64/libm.so.6 /lib64/librt.so.1 /lib64/libcap.so.2 /lib64/libdw.so.1 /lib64/libgcc_s.so.1 /lib64/libelf.so.1 /lib64/libz.so.1 /lib64/libbz2.so.1;do
	cp $lib jail/lib64/
	check_cp
done

for program in /bin/bash /bin/ls;do
	cp $program jail/bin
	check_cp
done

cp -R /usr/share/locale/en@piglatin jail/usr/share/locale ;check_cp

for script in grub2.sh grub2efi.sh;do
       cp $script jail/
       check_cp
done

cp -R /usr/sbin/grub2* jail/usr/sbin	;check_cp

exit 0
