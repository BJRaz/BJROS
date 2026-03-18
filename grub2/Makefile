# This makefile initiates a jail (chroot) for further procession of grub2
# to prevent curr

BUILDDIR=../build/x86
ISO=os.iso
ISO_DIR=iso
KERNEL=$(BUILDDIR)/kernel.elf

all:	$(ISO)

$(ISO):	$(KERNEL) | $(ISO_DIR)
	cp $(KERNEL) $(ISO_DIR)/boot/kernel.elf       		
	-cp -p grubiso.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub2-mkrescue -o $(ISO) $(ISO_DIR)
$(ISO_DIR):
	mkdir -p $(ISO_DIR)/boot/grub
clean:
	-rm -rf $(ISO_DIR)
	-rm $(ISO)
