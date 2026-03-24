# BJROS Copilot Instructions

BJROS is an experimental x86-32 bare-metal OS kernel written in C and NASM assembly. It targets 32-bit protected mode with a flat memory model and is multiboot-compliant (GRUB2 bootable).
The kernel is a microkernel in ring 0


## Build & Run

```bash
make              # Build kernel ELF → build/x86/kernel.elf
make grub2        # Build bootable ISO → grub2/os.iso
make clean        # Remove all build artifacts
make TAGS         # Regenerate ctags
DEBUG=1 make      # Build with -g debug symbols
```

```bash
make tests        # Build test executables in tests/
cd tests && ./test  # Run tests manually after building
```

```bash
./scripts/run-qemu-iso.sh             # Boot ISO in QEMU (headless, serial to stdout)
./scripts/run-qemu-iso.sh --gui       # Boot with GUI display
./scripts/run-qemu-iso.sh --gdb       # Boot with GDB stub (for remote debugging)
./scripts/run-qemu-iso.sh --iso PATH  # Use a custom ISO path
```

**Serial Console:** Kernel outputs to COM1 (0x3F8) and mirrors VGA text to serial. GRUB also outputs to serial. In headless mode, all kernel output appears on stdout.

**Toolchain:** Prefers `i386-elf-gcc` and `i386-elf-ld` cross-compilers if available in PATH. Falls back to host `gcc` and `ld`. On Linux, always uses `ld` directly to avoid libc injection. On macOS, uses compiler driver (`cc`) if cross-linker unavailable.
- Compilation: `gcc -m32 -nostdinc -ffreestanding -fno-stack-protector`
- Assembly: `nasm -felf32`
- Linking: `ld -m elf_i386` (or `gcc -m32` as fallback)

## Architecture

Boot flow: GRUB2 → `nasm/multiboot.asm` (multiboot header, entry point) → `kernel/kernel.c` (`kmain`) → hardware init → scheduler → idle loop.

**Design:** Microkernel-inspired with round-robin preemptive scheduler. Flat memory model (no MMU/paging). Console runs as a schedulable process. Keyboard input flows through ring buffers from ISR to process.

**Memory layout** (from `linker.ld`):
| Address    | Section  | Contents                        |
|------------|----------|---------------------------------|
| `0x100000` | `.text`  | Kernel code                     |
| `0x110000` | `.isr`   | Interrupt service routines      |
| `0x200000` | `.gdt`   | Global Descriptor Table         |
| `0x202000` | `.idt`   | Interrupt Descriptor Table      |
| `0x300000` | heap     | `_malloc`/`_free` heap (1 MB)   |
| `0x400000` | stacks   | Process stacks (4 KB × 8)       |

**Major components:**
- `nasm/multiboot.asm` — multiboot entry; receives bootloader info in `ebx`
- `nasm/boundaries.asm` — exposes kernel memory boundary symbols
- `kernel/kernel.c` — `kmain`: GDT/IDT init, PIC (8259) config, ISR wiring, scheduler init
- `kernel/console.c` — console process (PID 1), reads from keyboard ring buffer, VGA output
- `kernel/serial.c` — COM1 (0x3F8) serial driver; mirrors kernel output to serial port
- `kernel/ps2.c` — PS/2 controller init (keyboard + mouse); ports 0x60/0x64
- `kernel/ringbuf.c` — generic 256-byte circular ring buffer (ISR-safe single-producer/consumer)
- `kernel/process.c` — process control block (PCB), stack allocation, process creation
- `kernel/sched.c` — round-robin preemptive scheduler, timer-driven context switching
- `kernel/stdio/` — `kprintf`, `kprintln`, integer/string conversions
- `kernel/stdio/malloc.c` — `_malloc()`/`_free()` first-fit heap allocator
- `src/libc/` — freestanding libc (string, stdio); used by tests and kernel
- `tests/` — host-compiled unit tests for libc functions (not kernel tests)

The `.isr` section is placed at a fixed address so ISR function pointers can be hardcoded. Mark ISR functions with `ISR_FUNC` (`__attribute__((__section__(".isr")))`).

## Key Conventions

**Naming:**
- Kernel print functions: `kprintf()`, `kprintln()`, `kprint()`
- Internal libc functions use underscore prefix: `_strlen()`, `_strcmp()`, `_atoi()`, `_utoa()`, `_utox()`
- Macros: `UPPERCASE` (e.g., `VIDEO`, `IDT_FLAGS`, `ATTRIBUTE`)
- Assembly-callable C functions: `snake_case`

**Structs for hardware registers must be packed:**
```c
struct PACKED gdtr_register { ... };  // PACKED = __attribute__((packed))
```

**Inline assembly macros** (defined in `include/kernel/kernel.h`):
```c
#define i_cli  __asm__("cli");
#define i_sti  __asm__("sti");
#define i_hlt  __asm__("hlt");
```
Use these macros rather than raw `__asm__` strings for common instructions.

**I/O port access** via `inb`/`outb` (defined in kernel headers); busy-wait on status bits is the standard polling pattern:
```c
while (inb(PS2_CMD) & 0x2) {}  // wait for input buffer empty
```

**No standard library:** `#include` paths use `include/kernel` and `multiboot`. There is no libc available in kernel code — only the custom freestanding implementations in `src/libc/` and `kernel/stdio/`.

**Headers:** Kernel headers live in `include/kernel/` and `include/kernel/standard/`. Libc headers are in `include/libc/`. The compiler is invoked with `-Iinclude/kernel -Imultiboot`.

**Tests** are standalone host binaries (compiled without `-ffreestanding`) that test `src/libc/` functions in isolation. They are not run inside the kernel or QEMU.

## Debugging & Tools

**MCP Server** (`tools/mcp-qemu/`): A Model Context Protocol server that exposes QEMU and GDB control to AI assistants. Includes:
- QEMU tools: `start`, `stop`, `status`, `serial_read`, `send_key`, `monitor`
- GDB tools: `connect`, `continue`, `interrupt`, `break`, `delete_breakpoint`, `breakpoints`, `step`, `step_instruction`, `next`, `next_instruction`, `registers`, `memory`, `backtrace`, `disassemble`, `symbols`, `eval`

Dependencies: `mcp` (MCP SDK), `pygdbmi` (GDB/MI parser). See `tools/mcp-qemu/README.md` for setup and usage.
