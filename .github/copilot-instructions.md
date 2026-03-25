# BJROS Copilot Instructions

BJROS is an experimental x86-32 bare-metal OS kernel written in C and NASM assembly. It targets 32-bit protected mode with a flat memory model and is multiboot-compliant (GRUB2 bootable).
The kernel is a microkernel in ring 0.

## Project Structure

```
bjros/                         # Kernel and core OS code
├── kernel/                    # Kernel source (kernel.c, console.c, scheduler, etc.)
├── nasm/                      # Assembly source (multiboot.asm, ISRs, etc.)
├── include/kernel/            # Kernel headers
├── include/multiboot/         # Multiboot headers
├── src/libc/                  # Freestanding libc (string, stdio, stdlib functions)
├── include/libc/              # Libc headers
├── tests/                     # Host-compiled unit tests (test libc functions)
├── user/                      # User-space programs (TO BE IMPLEMENTED)
├── linker.ld                  # Kernel linker script
└── Makefile                   # Kernel build configuration
```

**Note:** The kernel resides in `bjros/` subdirectory. All kernel paths in this document are relative to `bjros/` or absolute from root.


## Build & Run

**Build commands (run from repository root):**

```bash
make              # Build kernel ELF → build/x86/kernel.elf
make grub2        # Build bootable ISO → grub2/os.iso
make clean        # Remove all build artifacts
make TAGS         # Regenerate ctags (in bjros/)
DEBUG=1 make      # Build with -g debug symbols
```

**Run tests (host-compiled unit tests in bjros/tests/):**

```bash
make tests        # Build test executables in bjros/tests/
cd bjros/tests && ./test  # Run tests manually after building
```

**Boot in QEMU:**

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

Boot flow: GRUB2 → `bjros/nasm/multiboot.asm` (multiboot header, entry point) → `bjros/kernel/kernel.c` (`kmain`) → hardware init → scheduler → idle loop.

**Design:** Microkernel-inspired with round-robin preemptive scheduler. Flat memory model (no MMU/paging). Console runs as a schedulable process. Keyboard input flows through ring buffers from ISR to process.

**Memory layout** (from `bjros/linker.ld`):
| Address    | Section  | Contents                        |
|------------|----------|---------------------------------|
| `0x100000` | `.text`  | Kernel code                     |
| `0x110000` | `.isr`   | Interrupt service routines      |
| `0x200000` | `.gdt`   | Global Descriptor Table         |
| `0x202000` | `.idt`   | Interrupt Descriptor Table      |
| `0x300000` | heap     | `_malloc`/`_free` heap (1 MB)   |
| `0x400000` | stacks   | Process stacks (4 KB × 8)       |

**Major components** (in `bjros/`):
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
- `user/` — user-space programs (reserved for future development)

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

**Inline assembly macros** (defined in `bjros/include/kernel/kernel.h`):
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

**No standard library:** `#include` paths use `include/kernel` and `multiboot` (relative to `bjros/`). There is no libc available in kernel code — only the custom freestanding implementations in `bjros/src/libc/` and `bjros/kernel/stdio/`.

**Headers:** Kernel headers live in `bjros/include/kernel/` and `bjros/include/kernel/standard/`. Libc headers are in `bjros/include/libc/`. The compiler is invoked with `-Iinclude/kernel -Imultiboot` (from within `bjros/`).

**Tests** are standalone host binaries (compiled without `-ffreestanding`) that test `bjros/src/libc/` functions in isolation. They are not run inside the kernel or QEMU. Build and run with `make tests` from root, then `cd bjros/tests && ./test`.

## User-Space Programs

**Directory:** `bjros/user/`

Currently reserved for user-space applications that will run in ring 3 (unprivileged mode). Planned features:
- User-space processes spawned by the kernel scheduler
- System calls to kernel services (IPC, memory management, etc.)
- Separation from kernel-space code and headers

Build configuration for user-space programs will be added as they are developed.

## Debugging & Tools

**MCP Server** (`tools/mcp-qemu/`): A Model Context Protocol server that exposes QEMU and GDB control to AI assistants. Includes:
- QEMU tools: `start`, `stop`, `status`, `serial_read`, `send_key`, `monitor`
- GDB tools: `connect`, `continue`, `interrupt`, `break`, `delete_breakpoint`, `breakpoints`, `step`, `step_instruction`, `next`, `next_instruction`, `registers`, `memory`, `backtrace`, `disassemble`, `symbols`, `eval`

Dependencies: `mcp` (MCP SDK), `pygdbmi` (GDB/MI parser). See `tools/mcp-qemu/README.md` for setup and usage.

**Kernel Source Structure:** Code is organized in `bjros/` for easy navigation:
- Kernel entry and core logic: `bjros/kernel/`
- Assembly routines: `bjros/nasm/`
- Build artifacts: `build/x86/` (kernel.elf) and `grub2/os.iso` (bootable ISO)
