CC ?= i386-elf-gcc
# If the environment default compiler is 'cc' (macOS) but an i386-elf cross
# compiler is available in PATH, prefer it so plain `make` works for cross-builds.
ifeq ($(CC),cc)
	ifneq ($(shell command -v i386-elf-gcc 2>/dev/null),)
		CC := i386-elf-gcc
	endif
endif

# Compiler flags for kernel (32-bit protected mode, bare metal)
CFLAGS=-nostdinc 			\
	-Wpadded 				\
	-std=c99 				\
	-m32 					\
	-c 						\
	-Wall 					\
	-ffreestanding 			\
	-fno-stack-protector 	\
	-Iinclude/kernel 		\
	-Iinclude/multiboot 	

AS=nasm
ASFLAGS=-felf32 

# Debug and release build variants
ifeq ($(DEBUG), 1)
	CFLAGS := $(CFLAGS) -g
	ASFLAGS := $(ASFLAGS) -Fdwarf
	BUILD_TYPE=debug
else
	BUILD_TYPE=release
endif

# Clang-specific settings
ifeq ($(CC), clang)
	CFLAGS := $(CFLAGS) -arch i386 		\
			-target i386-pc-none-elf 	\
			-v							\
			-nobuiltininc
endif

# Linker configuration
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

# Linker selection: use raw ld on Linux (avoids libc injection), 
# use compiler driver on macOS if cross-linker unavailable
LINK_CC_ARCH = -m32 
 
# Directories
OBJ_DIR:=../bin/x86
BUILD_DIR=../build/x86

# Object files: assembly and C sources
# Note: Assembly objects use .o extension (not .so, which is for shared libraries)
OBJS:=$(addprefix $(OBJ_DIR)/, \
	multiboot.o cursor.o atoi.o atou.o itoa.o utoa.o utox.o strlen.o strcmp.o \
	print.o console.o string.o serial.o ps2.o ringbuf.o malloc.o process.o sched.o kernel.o) 

VPATH=kernel:kernel/stdio:kernel/nasm:tests/stdio
vpath %.h include:include/multiboot 

# Build targets
all: $(BUILD_DIR)/kernel.elf TAGS

$(OBJS): | $(OBJ_DIR)							# order-only prerequisite

# Assembly rule: compile .asm files to .o objects
$(OBJ_DIR)/%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# C compilation rule: compile .c files to .o objects
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Directory creation
$(OBJ_DIR):
	-mkdir -p $(OBJ_DIR) 
$(BUILD_DIR):
	-mkdir -p $(BUILD_DIR)

# Platform-specific linker selection
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

# Kernel ELF build target
$(BUILD_DIR)/kernel.elf: $(OBJS) | $(BUILD_DIR)
	@echo "Linker: $(LINKER)"
	$(LINKER) $(LINKFLAGS) $^ -o $(BUILD_DIR)/kernel.elf
	-mbchk $@
clean:
	-rm -f tests/test 
	-rm -rf $(OBJ_DIR) $(BUILD_DIR)
	-cd tests && $(MAKE) clean
	-cd src/libc && $(MAKE) clean

# Tags generation for editor navigation (ctags)
TAGS:	
	ctags --exclude=build/ --exclude=bin/ --exclude=*.so -R .

# Phase 2: Build variants and information targets
.PHONY: debug release info show-config tests TAGS

# Debug build: compiles with -g symbols for GDB
debug: DEBUG=1
debug: all

# Release build: stripped, optimized (future enhancement)
release: CFLAGS += -O2
release: all

# Display build configuration
info:
	@echo "BJROS Kernel Build Configuration"
	@echo "================================="
	@echo "Build Type:  $(BUILD_TYPE)"
	@echo "Compiler:    $(CC) ($(shell $(CC) --version | head -1))"
	@echo "Assembler:   $(AS)"
	@echo "Linker:      $(LINKER)"
	@echo "Platform:    $(OS)"
	@echo ""
	@echo "CFLAGS:      $(CFLAGS)"
	@echo "ASFLAGS:     $(ASFLAGS)"
	@echo "LD_FLAGS:    $(LINKFLAGS)"
	@echo ""
	@echo "Object Dir:  $(OBJ_DIR)"
	@echo "Build Dir:   $(BUILD_DIR)"
	@echo "Output:      $(BUILD_DIR)/kernel.elf"

# Show build system configuration
show-config: info

# Phase 3: Libc and tests support
export CC CFLAGS AS ASFLAGS OBJS OBJ_DIR

$(OBJ_DIR)/libc.o: 
	cd src/libc && $(MAKE)
	cp src/libc/libc.o $(OBJ_DIR)/libc.o

tests:	$(OBJS) $(OBJ_DIR)/libc.o	
	cd tests && $(MAKE)
