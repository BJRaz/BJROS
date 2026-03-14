CC ?= i386-elf-gcc
# If the environment default compiler is 'cc' (macOS) but an i386-elf cross
# compiler is available in PATH, prefer it so plain `make` works for cross-builds.
ifeq ($(CC),cc)
	ifneq ($(shell command -v i386-elf-gcc 2>/dev/null),)
		CC := i386-elf-gcc
	endif
endif
CFLAGS=-nostdinc 		\
	-Wpadded 		\
	-std=c99 		\
	-m32 			\
	-c 			\
	-Wall 			\
	-ffreestanding 		\
	-fno-stack-protector 	\
	-Iinclude/kernel 	\
	-Imultiboot 
AS=nasm
ASFLAGS=-felf32 

ifeq ($(DEBUG), 1)
	CFLAGS := $(CFLAGS) -g
	ASFLAGS := $(ASFLAGS) -Fdwarf
endif

ifeq ($(CC), clang)
	CFLAGS := $(CFLAGS) -arch i386 			\
			-target i386-pc-none-elf 	\
			-v				\
			-nobuiltininc	# clang specific option
endif

LD ?= i386-elf-ld
ifeq ($(LD),ld)
	ifneq ($(shell command -v i386-elf-ld 2>/dev/null),)
		LD := i386-elf-ld
	endif
endif
LD_ARCH_FLAG = -m elf_i386
LD_FLAGS = -L bin \
	-T linker.ld \
	-static \
	-z muldefs

# When linking with the compiler driver we need an architecture flag suitable
# for gcc (use -m32); when using the raw cross-linker we use the ld-specific
# `-m elf_i386` flag.
LINK_CC_ARCH = -m32
# **** 
# C++ settings
# LDFLAGS=-m elf_i386 -T linker.ld -lstdc++ -L /usr/lib/gcc/i686-redhat-linux/10 --static #/usr/lib/crt1.o 
# ****
 

OBJDIR:=bin/x86
#OBJS:=$(addprefix $(OBJDIR)/, multiboot.so string.o cursor.so print.o console.o ps2.o kernel.o) 
OBJS:=$(addprefix $(OBJDIR)/, multiboot.so cursor.so atoi.so atou.so itoa.so utoa.so utox.so strlen.so strcmp.so print.o console.o string.o ps2.o spinlock.o kernel.o) 
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
	-mkdir -p $(OBJDIR) 
$(BUILDDIR):
	-mkdir -p $(BUILDDIR)

OS := $(shell uname -s)

# On Linux we must call the linker (ld) explicitly; using the compiler driver
# (`cc`) as the linker injects host libraries like -lc which breaks kernel linking.
ifeq ($(OS),Linux)
LINKER := $(LD)
LINKFLAGS := $(LD_ARCH_FLAG) $(LD_FLAGS)
else
# On non-Linux (macOS) prefer the cross-linker when available, otherwise use CC
ifeq ($(shell basename $(LD)),i386-elf-ld)
LINKER := $(LD)
LINKFLAGS := $(LD_ARCH_FLAG) $(LD_FLAGS)
else
LINKER := $(CC)
LINKFLAGS := $(LINK_CC_ARCH) $(LD_FLAGS)
endif
endif

$(BUILDDIR)/kernel.elf: $(OBJS) | $(BUILDDIR)
	@echo "Linker: $(LINKER)"
	$(LINKER) $(LINKFLAGS) $^ -o $(BUILDDIR)/kernel.elf
	-mbchk $@
clean:
	-rm -f tests/test 
	-rm -rf $(OBJDIR) $(BUILDDIR)
	-cd grub2 && $(MAKE) clean
	-cd tests && $(MAKE) clean
	-cd src/libc && $(MAKE) clean
TAGS:
	@# Try to generate tags; prefer long-option ctags, but fall back to simple recursive ctags
	@ctags --exclude=multiboot/kernel.c --exclude=kernel/k.c --exclude=jail/ -R . 2>/dev/null || \
	ctags -R . 2>/dev/null || true
export CC CFLAGS AS ASFLAGS OBJS OBJDIR

.PHONY:	tests grub2

$(OBJDIR)/libc.o: 
	cd src/libc && $(MAKE)
	cp src/libc/libc.o $(OBJDIR)/libc.o
tests:	$(OBJS) $(OBJDIR)/libc.o	
	cd tests && $(MAKE)
grub2:	$(BUILDDIR)/kernel.elf
	cd grub2 && $(MAKE) #-f Makefile.grub2efi
