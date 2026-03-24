# mcp-qemu — MCP Server for QEMU + GDB Debugging

An [MCP (Model Context Protocol)](https://modelcontextprotocol.io/) server that exposes QEMU lifecycle management and GDB debugging as tools, enabling AI assistants to interactively boot, inspect, and debug the BJROS kernel.

## Quick Start

```bash
cd tools/mcp-qemu
python3 -m venv .venv
source .venv/bin/activate
pip install mcp pygdbmi
python server.py          # starts MCP server on stdio
```

## MCP Client Configuration

### VS Code / Copilot

Add to your MCP settings (`.vscode/mcp.json` or user settings):

```json
{
  "servers": {
    "bjros-qemu": {
      "type": "stdio",
      "command": "python",
      "args": ["server.py"],
      "cwd": "tools/mcp-qemu",
      "env": {
        "PATH": "tools/mcp-qemu/.venv/bin:${env:PATH}"
      }
    }
  }
}
```

### Claude Desktop

Add to `claude_desktop_config.json`:

```json
{
  "mcpServers": {
    "bjros-qemu": {
      "command": "/path/to/bjros/tools/mcp-qemu/.venv/bin/python",
      "args": ["/path/to/bjros/tools/mcp-qemu/server.py"]
    }
  }
}
```

## Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `BJROS_KERNEL_ELF` | `build/x86/kernel.elf` | Kernel ELF with debug symbols |
| `BJROS_KERNEL_ISO` | `grub2/os.iso` | Bootable GRUB2 ISO |
| `QEMU_BIN` | `qemu-system-i386` | QEMU binary path |
| `QEMU_MEMORY` | `128M` | Guest RAM |
| `QEMU_GDB_PORT` | `1234` | GDB stub TCP port |
| `GDB_BIN` | `gdb` | GDB binary path |
| `GDB_CONNECT_TIMEOUT` | `10` | GDB command timeout (seconds) |

## Tools Reference

### QEMU Lifecycle

| Tool | Description |
|------|-------------|
| `qemu_start` | Boot kernel ISO in QEMU (headless, paused, GDB stub enabled) |
| `qemu_stop` | Terminate QEMU (disconnects GDB first) |
| `qemu_status` | Check if running, PID, uptime, serial buffer size |
| `qemu_serial_read` | Read recent serial output (lines param, default 50) |
| `qemu_send_key` | Send keystroke to guest (`ret`, `a`, `ctrl-c`, etc.) |
| `qemu_monitor` | Send arbitrary QEMU monitor command |

### GDB Debugging

| Tool | Description |
|------|-------------|
| `gdb_connect` | Attach GDB to QEMU, load kernel symbols |
| `gdb_continue` | Resume execution |
| `gdb_interrupt` | Pause execution |
| `gdb_break` | Set breakpoint (symbol, file:line, or *0xADDR) |
| `gdb_delete_breakpoint` | Delete breakpoint by number |
| `gdb_breakpoints` | List all breakpoints |
| `gdb_step` | Step one source line (into calls) |
| `gdb_step_instruction` | Step one machine instruction |
| `gdb_next` | Step over source line |
| `gdb_next_instruction` | Step over machine instruction |
| `gdb_registers` | Read all CPU registers (hex) |
| `gdb_memory` | Read N bytes at address (hex dump) |
| `gdb_backtrace` | Show call stack |
| `gdb_disassemble` | Disassemble at address/symbol/PC |
| `gdb_symbols` | Load symbol file |
| `gdb_eval` | Evaluate GDB expression |

## Typical Workflow

```
1. qemu_start()                    → boots BJROS ISO, paused at BIOS
2. gdb_connect()                   → GDB attaches, loads symbols
3. gdb_break("kmain")              → breakpoint at kernel entry
4. gdb_continue()                  → GRUB boots, kernel starts, hits breakpoint
5. gdb_registers()                 → inspect CPU state at kmain
6. gdb_disassemble()               → view instructions at PC
7. gdb_memory("0xb8000", 160)      → read VGA text buffer (first row)
8. qemu_serial_read()              → read serial boot messages
9. gdb_continue()                  → let kernel run
10. qemu_send_key("ret")           → press Enter in guest
11. qemu_serial_read()             → see kernel response
12. qemu_stop()                    → shut down
```

## Architecture

```
┌─────────────┐  JSON-RPC/stdio  ┌─────────────────┐
│  AI Client  │ ◄──────────────► │  MCP Server      │
│  (Copilot)  │                  │  server.py       │
└─────────────┘                  └──┬──────────┬────┘
                                    │          │
                           subprocess│    GDB/MI│
                                    ▼          ▼
                                ┌───────┐  ┌──────┐
                                │ QEMU  │◄─│ GDB  │
                                │ i386  │  │      │
                                └───────┘  └──────┘
```

- `server.py` — MCP entry point, registers all tools
- `qemu_manager.py` — QEMU subprocess lifecycle, serial capture, monitor socket
- `gdb_controller.py` — pygdbmi wrapper for GDB/MI commands
- `config.py` — paths, ports, timeouts (overridable via env vars)
