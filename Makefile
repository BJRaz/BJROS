CC=clang
CFLAGS=-nostdinc -Wpadded -std=c99 -m32 -c -Wall -ffreestanding -fno-stack-protector -Iinclude -Imultiboot 
AS=nasm
ASFLAGS=-felf32 

ifeq ($(DEBUG), 1)
	CFLAGS := $(CFLAGS) -g
	ASFLAGS := $(ASFLAGS) -Fdwarf
endif

ifeq ($(CC), clang)
	CFLAGS := $(CFLAGS) -arch i386 -target i386-pc-none-elf -nobuiltininc	# clang specific option
endif

LD=ld
LDFLAGS=-m elf_i386 -L bin -T linker.ld -static 
#LDFLAGS=-m elf_i386 -T linker.ld -lstdc++ -L /usr/lib/gcc/i686-redhat-linux/10 --static #/usr/lib/crt1.o 
#-M 
OBJDIR:=bin
OBJS:=$(addprefix $(OBJDIR)/, multiboot.so cursor.so atoi.so atou.so itoa.so utoa.so utox.so strlen.so print.o console.o string.so kernel.o) 

LODEV=/dev/loop0
# settings floppy
GRUBFILE=grub_legacy/setup_grub.txt
OUTPUT=/media/sf_VBoxLinuxShare/binaries/floppy.img	# TODO path remove etc...
IMG=floppy.img
MOUNTPOINT=/mnt/floppy

#settings HDD
GRUBFILEHD=grub_legacy/setup_grub_hd.txt
OUTPUTHD=/media/sf_VBoxLinuxShare/binaries/hdd.img
IMGHD=hdd.img
LODEVHD=/dev/mapper/loop0p1

VPATH=kernel:kernel/stdio:nasm:tests/stdio		# make searchdirs variable...
vpath %.h include					# search for specific filetypes in <dir>

all: kernel.elf TAGS

$(OBJS): | $(OBJDIR)					# order-only prerequisite

# had to make this rule match *.so (shared object) 
# when referencing assembly files
$(OBJDIR)/%.so: %.asm
	$(AS) $(ASFLAGS) $< -o $@
# this matches all c-files
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@


$(OBJDIR):
	-mkdir $(OBJDIR) 
kernel.elf: $(OBJS)  
	$(LD) $(LDFLAGS) $^ -o kernel.elf
	-mbchk $@
clean:
	-rm -f -r $(OBJS) kernel.o
	-rm -f $(IMG)	
	-rm -f kernel.elf
	-rm -f test 
#	-rm -f tags
	-rm -f $(IMGHD) 
	-rm -rf $(OBJDIR)
bochs: install
	bochs "boot:floppy" "floppya: 1_44=floppy.img, status=inserted"
qemu: install
	qemu -fda floppy.img 
TAGS:
	-ctags .
tests:	$(OBJS) itoa.c 
	#$(CC) -g -I. -o test $(filter-out $(OBJDIR)/multiboot.so $(OBJDIR)/kernel.o, $^) 
	$(CC) -m32 -I. -g tests/stdio/itoa.c -o test bin/atoi.so bin/atou.so bin/utoa.so bin/itoa.so bin/utox.so bin/strlen.so bin/string.so 
