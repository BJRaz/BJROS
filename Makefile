.DEFAULT_GOAL := all

.PHONY: all bjros grub2 clean distclean tests run gdb help info show-toolchain

help:
	@echo "BJROS Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  make all        Build kernel ELF and bootable ISO"
	@echo "  make clean      Remove build artifacts (kernel/GRUB)"
	@echo "  make distclean  Full cleanup (includes build directories)"
	@echo "  make tests      Build and run unit tests"
	@echo "  make run        Boot ISO in QEMU (headless, serial to stdout)"
	@echo "  make gdb        Boot ISO in QEMU with GDB stub (port 1234)"
	@echo "  make info       Show build configuration"
	@echo "  make show-toolchain  Detect and display toolchain"
	@echo "  make help       Show this help message"
	@echo ""
	@echo "Build options:"
	@echo "  DEBUG=1         Compile with debug symbols (-g)"
	@echo "  CC=compiler     Override C compiler (default: i386-elf-gcc)"
	@echo "  LD=linker       Override linker (default: i386-elf-ld)"
	@echo ""
	@echo "Examples:"
	@echo "  make                # Build kernel and ISO (default)"
	@echo "  make DEBUG=1 run    # Debug build, boot in QEMU"
	@echo "  make clean && make  # Full rebuild"

all: bjros grub2

bjros:	
	cd bjros && $(MAKE)

grub2: bjros	
	cd grub2 && $(MAKE)

clean:
	cd bjros && $(MAKE) clean
	cd grub2 && $(MAKE) clean

distclean: clean
	rm -rf build/ bjros/build/ bjros/bin/

tests:
	cd bjros && $(MAKE) tests

run: grub2
	./scripts/run-qemu-iso.sh

gdb: grub2
	./scripts/run-qemu-iso.sh --gdb

info:
	@echo "BJROS Build Configuration"
	@echo "========================="
	@echo "Kernel ELF: build/x86/kernel.elf"
	@echo "Bootable ISO: grub2/os.iso"
	@cd bjros && $(MAKE) info || echo "(kernel info available from bjros/Makefile)"

show-toolchain:
	@echo "Detected Toolchain"
	@echo "=================="
	@which i386-elf-gcc > /dev/null 2>&1 && echo "✓ i386-elf-gcc (cross-compiler)" || echo "✗ i386-elf-gcc (NOT found)"
	@which i386-elf-ld > /dev/null 2>&1 && echo "✓ i386-elf-ld (cross-linker)" || echo "✗ i386-elf-ld (NOT found)"
	@which nasm > /dev/null 2>&1 && echo "✓ nasm (assembler)" || echo "✗ nasm (NOT found)"
	@which grub2-mkrescue > /dev/null 2>&1 && echo "✓ grub2-mkrescue (ISO builder)" || echo "✗ grub2-mkrescue (NOT found)"
	@which gdb > /dev/null 2>&1 && echo "✓ gdb (debugger)" || echo "✗ gdb (NOT found)"
	@which qemu-system-i386 > /dev/null 2>&1 && echo "✓ qemu-system-i386 (emulator)" || echo "✗ qemu-system-i386 (NOT found)"
	@echo ""
	@echo "Current toolchain:"
	@echo "CC: $$(cc --version | head -1)"
	@echo "LD: $$(ld --version | head -1)"

