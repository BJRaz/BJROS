"""MCP Server for QEMU + GDB debugging of BJROS kernel.

Usage:
    cd tools/mcp-qemu
    source .venv/bin/activate
    python server.py
"""

from mcp.server.fastmcp import FastMCP

from qemu_manager import QemuManager
from gdb_controller import GdbController_

mcp = FastMCP(
    name="mcp-qemu",
    instructions=(
        "QEMU lifecycle management and GDB debugging server for the BJROS x86-32 kernel. "
        "Start QEMU, connect GDB, set breakpoints, step through code, inspect registers and memory."
    ),
)

qemu = QemuManager()
gdb = GdbController_()


# =====================================================================
# QEMU Lifecycle Tools
# =====================================================================

@mcp.tool(description="Boot the BJROS kernel ISO in QEMU (headless, paused, GDB stub on port 1234). Returns PID and GDB port.")
async def qemu_start(iso_path: str | None = None, extra_args: str | None = None) -> dict:
    args = extra_args.split() if extra_args else None
    return await qemu.start(iso_path=iso_path, extra_args=args)


@mcp.tool(description="Terminate the running QEMU instance.")
async def qemu_stop() -> dict:
    # Disconnect GDB first if connected
    if gdb.connected:
        await gdb.disconnect()
    return await qemu.stop()


@mcp.tool(description="Check QEMU status: running, PID, uptime, GDB port, serial buffer size.")
async def qemu_status() -> dict:
    return qemu.status()


@mcp.tool(description="Read recent serial console output from the kernel. `lines` controls how many lines to return (default 50).")
async def qemu_serial_read(lines: int = 50) -> dict:
    return qemu.serial_read(lines=lines)


@mcp.tool(description="Send a keystroke to the QEMU guest. Key names follow QEMU sendkey syntax: 'a', 'ret', 'ctrl-c', 'shift-a', etc.")
async def qemu_send_key(key: str) -> dict:
    return await qemu.send_key(key)


@mcp.tool(description="Send an arbitrary command to the QEMU monitor (e.g., 'info registers', 'info mem', 'xp /16x 0x100000').")
async def qemu_monitor(command: str) -> dict:
    return await qemu.send_monitor_command(command)


# =====================================================================
# GDB Debugging Tools
# =====================================================================

@mcp.tool(description="Connect GDB to the running QEMU instance and load kernel symbols. Call after qemu_start.")
async def gdb_connect(symbol_file: str | None = None) -> dict:
    return await gdb.connect(symbol_file=symbol_file)


@mcp.tool(description="Resume kernel execution (GDB continue).")
async def gdb_continue() -> dict:
    return await gdb.continue_exec()


@mcp.tool(description="Pause kernel execution (send interrupt to GDB).")
async def gdb_interrupt() -> dict:
    return await gdb.interrupt()


@mcp.tool(description="Set a breakpoint. Location can be a function name (e.g. 'kmain'), file:line (e.g. 'k.c:42'), or address ('*0x100000').")
async def gdb_break(location: str) -> dict:
    return await gdb.breakpoint_set(location)


@mcp.tool(description="Delete a breakpoint by its number.")
async def gdb_delete_breakpoint(number: int) -> dict:
    return await gdb.breakpoint_delete(number)


@mcp.tool(description="List all breakpoints.")
async def gdb_breakpoints() -> dict:
    return await gdb.breakpoint_list()


@mcp.tool(description="Step one machine instruction (stepi). Use for precise instruction-level stepping.")
async def gdb_step_instruction() -> dict:
    return await gdb.step_instruction()


@mcp.tool(description="Step one source line (step). Steps into function calls.")
async def gdb_step() -> dict:
    return await gdb.step()


@mcp.tool(description="Step over one machine instruction (nexti). Skips over calls.")
async def gdb_next_instruction() -> dict:
    return await gdb.next_instruction()


@mcp.tool(description="Step over one source line (next). Skips over function calls.")
async def gdb_next() -> dict:
    return await gdb.next()


@mcp.tool(description="Read all CPU register values (hex format).")
async def gdb_registers() -> dict:
    return await gdb.registers()


@mcp.tool(description="Read memory at address. Returns hex dump. address: hex address (e.g. '0xb8000'). count: bytes to read (default 64).")
async def gdb_memory(address: str, count: int = 64) -> dict:
    return await gdb.memory_read(address, count)


@mcp.tool(description="Show the call stack (backtrace).")
async def gdb_backtrace() -> dict:
    return await gdb.backtrace()


@mcp.tool(description="Disassemble instructions. start: address or symbol (default: current PC). count: number of instructions (default 20).")
async def gdb_disassemble(start: str | None = None, end: str | None = None, count: int = 20) -> dict:
    return await gdb.disassemble(start=start, end=end, count=count)


@mcp.tool(description="Load a symbol file into GDB (default: build/x86/kernel.elf).")
async def gdb_symbols(path: str | None = None) -> dict:
    return await gdb.load_symbols(path)


@mcp.tool(description="Evaluate an arbitrary GDB expression (e.g., '$eax', '*(int*)0x100000', 'sizeof(struct gdtr_register)').")
async def gdb_eval(expression: str) -> dict:
    return await gdb.evaluate(expression)


# =====================================================================
# Entry point
# =====================================================================

def main():
    mcp.run(transport="stdio")


if __name__ == "__main__":
    main()
