CC=gcc
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
AS=nasm
ASFLAGS=-felf32 #-Fdwarf   
OBJDIR:=bin
OBJS:=$(addprefix $(OBJDIR)/, multiboot.so cursor.so atoi.so atou.so itoa.so utoa.so utox.so strlen.so print.o console.o string.so kernel.o) 
BUILDDIR=build/x86

VPATH=kernel:kernel/stdio:nasm:tests/stdio		# make searchdirs variable...
vpath %.h include					# search for specific filetypes in <dir>

all: $(BUILDDIR)/kernel.elf TAGS

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
$(BUILDDIR):
	-mkdir -p $(BUILDDIR)
$(BUILDDIR)/kernel.elf: $(OBJS) | $(BUILDDIR)  
	$(LD) $(LDFLAGS) $^ -o $(BUILDDIR)/kernel.elf
	-mbchk $@
clean:
	-rm -f tests/test 
	-rm -rf $(OBJDIR) $(BUILDDIR)
	-cd grub2 && $(MAKE) clean
TAGS:
	ctags --exclude=kernel/k.c --exclude=jail/ -R .
export CC OBJS OBJDIR

.PHONY:	tests grub2

tests:	$(OBJS)	
	cd tests && $(MAKE)
grub2:	$(BUILDDIR)/kernel.elf
	cd grub2 && $(MAKE)
