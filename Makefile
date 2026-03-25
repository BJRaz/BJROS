BUILDDIR=build/x86

.PHONY:	bjros grub2

bjros:	
	cd bjros && $(MAKE)
grub2:	bjros	
	cd grub2 && $(MAKE) 
clean:
	cd bjros && $(MAKE) clean
	cd grub2 && $(MAKE) clean

