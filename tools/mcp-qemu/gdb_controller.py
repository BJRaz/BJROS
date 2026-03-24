"""GDB/MI controller using pygdbmi.

Wraps common GDB debugging commands and translates GDB/MI responses
into simple dicts suitable for MCP tool returns.
"""

import asyncio
from functools import partial

from pygdbmi.gdbcontroller import GdbController

import config


class GdbController_:
    """Async-friendly wrapper around pygdbmi's GdbController."""

    def __init__(self) -> None:
        self._gdb: GdbController | None = None
        self._connected: bool = False
        self._symbol_file: str | None = None

    # ------------------------------------------------------------------
    # Connection
    # ------------------------------------------------------------------

    async def connect(
        self,
        host: str = "localhost",
        port: int | None = None,
        symbol_file: str | None = None,
    ) -> dict:
        """Start GDB and connect to the QEMU GDB stub."""
        if self._connected and self._gdb:
            return {"ok": True, "status": "already connected"}

        port = port or config.QEMU_GDB_PORT
        elf = symbol_file or config.KERNEL_ELF
        self._symbol_file = elf

        loop = asyncio.get_event_loop()
        try:
            self._gdb = await loop.run_in_executor(
                None,
                partial(GdbController, command=[config.GDB_BIN, "--interpreter=mi3", "-q"]),
            )
            # Load symbols
            await self._mi(f"-file-exec-and-symbols {elf}")
            # Connect to remote
            resp = await self._mi(f"-target-select remote {host}:{port}")
            self._connected = True
            return {"ok": True, "target": f"{host}:{port}", "symbols": elf, "response": _extract_payload(resp)}
        except Exception as exc:
            self._gdb = None
            self._connected = False
            return {"ok": False, "error": str(exc)}

    async def disconnect(self) -> dict:
        """Detach and quit GDB."""
        if not self._gdb:
            return {"ok": True, "status": "not connected"}
        try:
            await self._mi("-target-detach")
        except Exception:
            pass
        try:
            await self._mi("-gdb-exit")
        except Exception:
            pass
        self._gdb = None
        self._connected = False
        return {"ok": True}

    # ------------------------------------------------------------------
    # Execution control
    # ------------------------------------------------------------------

    async def continue_exec(self) -> dict:
        """Resume execution."""
        resp = await self._mi("-exec-continue")
        return {"ok": True, "response": _extract_payload(resp)}

    async def interrupt(self) -> dict:
        """Pause execution (send interrupt to inferior)."""
        resp = await self._mi("-exec-interrupt")
        return {"ok": True, "response": _extract_payload(resp)}

    async def step_instruction(self) -> dict:
        """Step one machine instruction (si)."""
        resp = await self._mi("-exec-step-instruction")
        return {"ok": True, "response": _extract_payload(resp)}

    async def step(self) -> dict:
        """Step one source line (s)."""
        resp = await self._mi("-exec-step")
        return {"ok": True, "response": _extract_payload(resp)}

    async def next_instruction(self) -> dict:
        """Step over one machine instruction (ni)."""
        resp = await self._mi("-exec-next-instruction")
        return {"ok": True, "response": _extract_payload(resp)}

    async def next(self) -> dict:
        """Step over one source line (n)."""
        resp = await self._mi("-exec-next")
        return {"ok": True, "response": _extract_payload(resp)}

    # ------------------------------------------------------------------
    # Breakpoints
    # ------------------------------------------------------------------

    async def breakpoint_set(self, location: str) -> dict:
        """Set a breakpoint. Location can be a symbol name, file:line, or *0xADDR."""
        resp = await self._mi(f"-break-insert {location}")
        return {"ok": True, "response": _extract_payload(resp)}

    async def breakpoint_delete(self, number: int) -> dict:
        """Delete a breakpoint by number."""
        resp = await self._mi(f"-break-delete {number}")
        return {"ok": True, "response": _extract_payload(resp)}

    async def breakpoint_list(self) -> dict:
        """List all breakpoints."""
        resp = await self._mi("-break-list")
        return {"ok": True, "response": _extract_payload(resp)}

    # ------------------------------------------------------------------
    # Inspection
    # ------------------------------------------------------------------

    async def registers(self) -> dict:
        """Read all register values."""
        # Get register names first
        names_resp = await self._mi("-data-list-register-names")
        values_resp = await self._mi("-data-list-register-values x")
        return {
            "ok": True,
            "names": _extract_payload(names_resp),
            "values": _extract_payload(values_resp),
        }

    async def memory_read(self, address: str, count: int = 64) -> dict:
        """Read `count` bytes at `address` (hex format)."""
        resp = await self._mi(f"-data-read-memory-bytes {address} {count}")
        return {"ok": True, "response": _extract_payload(resp)}

    async def backtrace(self) -> dict:
        """Get the call stack."""
        resp = await self._mi("-stack-list-frames")
        return {"ok": True, "response": _extract_payload(resp)}

    async def disassemble(self, start: str | None = None, end: str | None = None, count: int = 20) -> dict:
        """Disassemble instructions. With no args, disassemble around PC."""
        if start and end:
            resp = await self._mi(f"-data-disassemble -s {start} -e {end} -- 0")
        elif start:
            # Use a console command for N instructions from address
            resp = await self._mi(f'-interpreter-exec console "x/{count}i {start}"')
        else:
            resp = await self._mi(f'-interpreter-exec console "x/{count}i $pc"')
        return {"ok": True, "response": _extract_payload(resp)}

    async def evaluate(self, expression: str) -> dict:
        """Evaluate a GDB expression."""
        resp = await self._mi(f"-data-evaluate-expression {expression}")
        return {"ok": True, "response": _extract_payload(resp)}

    async def load_symbols(self, path: str | None = None) -> dict:
        """Load symbol file."""
        elf = path or config.KERNEL_ELF
        resp = await self._mi(f"-file-exec-and-symbols {elf}")
        self._symbol_file = elf
        return {"ok": True, "file": elf, "response": _extract_payload(resp)}

    # ------------------------------------------------------------------
    # Internals
    # ------------------------------------------------------------------

    async def _mi(self, command: str) -> list[dict]:
        """Send a GDB/MI command and return parsed response."""
        if not self._gdb:
            raise RuntimeError("GDB not connected")
        loop = asyncio.get_event_loop()
        return await loop.run_in_executor(
            None,
            partial(self._gdb.write, command, timeout_sec=config.GDB_CONNECT_TIMEOUT),
        )

    @property
    def connected(self) -> bool:
        return self._connected


def _extract_payload(responses: list[dict]) -> list[dict]:
    """Extract meaningful payload from pygdbmi response list.

    Each response dict has keys: type, message, payload, token, stream.
    We filter out log/console noise and return the useful entries.
    """
    results = []
    for r in responses:
        if r.get("type") == "result" or (r.get("type") == "console" and r.get("payload")):
            results.append({
                "type": r.get("type"),
                "message": r.get("message"),
                "payload": r.get("payload"),
            })
    return results
