"""QEMU subprocess lifecycle manager.

Manages a single QEMU instance: start, stop, status, serial I/O,
and guest keystrokes via the QEMU monitor.
"""

import asyncio
import os
import signal
import socket
import time
from collections import deque
from pathlib import Path

import config


class QemuManager:
    """Singleton-style manager for a QEMU subprocess."""

    def __init__(self) -> None:
        self._proc: asyncio.subprocess.Process | None = None
        self._serial_buf: deque[str] = deque(maxlen=4096)
        self._start_time: float | None = None
        self._reader_task: asyncio.Task | None = None
        self._monitor_sock_path: str = config.QEMU_MONITOR_SOCK

    # ------------------------------------------------------------------
    # Lifecycle
    # ------------------------------------------------------------------

    async def start(
        self,
        iso_path: str | None = None,
        extra_args: list[str] | None = None,
    ) -> dict:
        """Launch QEMU with GDB stub, serial on stdio, monitor on unix socket."""
        if self._proc is not None and self._proc.returncode is None:
            return {"ok": False, "error": "QEMU already running", "pid": self._proc.pid}

        iso = iso_path or config.KERNEL_ISO
        if not Path(iso).exists():
            return {"ok": False, "error": f"ISO not found: {iso}"}

        # Clean up stale monitor socket
        if os.path.exists(self._monitor_sock_path):
            os.unlink(self._monitor_sock_path)

        cmd = [
            config.QEMU_BIN,
            "-m", config.QEMU_MEMORY,
            "-cdrom", iso,
            "-boot", "d",
            "-nographic",
            "-gdb", f"tcp::{config.QEMU_GDB_PORT}",
            "-S",  # start paused (waiting for GDB continue)
            "-monitor", f"unix:{self._monitor_sock_path},server,nowait",
        ]
        if extra_args:
            cmd.extend(extra_args)

        self._serial_buf.clear()
        self._proc = await asyncio.create_subprocess_exec(
            *cmd,
            stdin=asyncio.subprocess.PIPE,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
        )
        self._start_time = time.time()

        # Background task to read serial output
        self._reader_task = asyncio.create_task(self._read_serial())

        return {"ok": True, "pid": self._proc.pid, "gdb_port": config.QEMU_GDB_PORT}

    async def stop(self) -> dict:
        """Terminate the running QEMU instance."""
        if self._proc is None or self._proc.returncode is not None:
            return {"ok": False, "error": "QEMU is not running"}

        self._proc.terminate()
        try:
            await asyncio.wait_for(self._proc.wait(), timeout=5)
        except asyncio.TimeoutError:
            self._proc.kill()
            await self._proc.wait()

        if self._reader_task:
            self._reader_task.cancel()
            try:
                await self._reader_task
            except asyncio.CancelledError:
                pass

        pid = self._proc.pid
        self._proc = None
        self._start_time = None

        # Clean up monitor socket
        if os.path.exists(self._monitor_sock_path):
            os.unlink(self._monitor_sock_path)

        return {"ok": True, "pid": pid}

    def status(self) -> dict:
        """Check if QEMU is running."""
        if self._proc is None or self._proc.returncode is not None:
            return {"running": False}
        return {
            "running": True,
            "pid": self._proc.pid,
            "uptime_seconds": round(time.time() - self._start_time, 1) if self._start_time else 0,
            "gdb_port": config.QEMU_GDB_PORT,
            "serial_lines_buffered": len(self._serial_buf),
        }

    # ------------------------------------------------------------------
    # Serial I/O
    # ------------------------------------------------------------------

    async def _read_serial(self) -> None:
        """Background coroutine: read QEMU stdout (serial) into buffer."""
        assert self._proc and self._proc.stdout
        try:
            while True:
                line = await self._proc.stdout.readline()
                if not line:
                    break
                self._serial_buf.append(line.decode("utf-8", errors="replace"))
        except asyncio.CancelledError:
            pass

    def serial_read(self, lines: int = 50) -> dict:
        """Return the last N lines of serial output."""
        buf = list(self._serial_buf)
        recent = buf[-lines:] if lines < len(buf) else buf
        return {"ok": True, "lines": recent, "total_buffered": len(buf)}

    # ------------------------------------------------------------------
    # QEMU Monitor (for sendkey, etc.)
    # ------------------------------------------------------------------

    async def send_key(self, key: str) -> dict:
        """Send a keystroke to the guest via the QEMU monitor socket.

        Key names follow QEMU `sendkey` syntax, e.g. 'a', 'ret', 'ctrl-c'.
        """
        if self._proc is None or self._proc.returncode is not None:
            return {"ok": False, "error": "QEMU is not running"}

        try:
            reader, writer = await asyncio.open_unix_connection(self._monitor_sock_path)
            # Consume the initial prompt/banner
            await asyncio.wait_for(reader.read(4096), timeout=1)

            writer.write(f"sendkey {key}\n".encode())
            await writer.drain()
            await asyncio.sleep(0.1)
            resp = await asyncio.wait_for(reader.read(4096), timeout=1)
            writer.close()
            await writer.wait_closed()
            return {"ok": True, "key": key, "response": resp.decode(errors="replace").strip()}
        except (ConnectionRefusedError, FileNotFoundError) as exc:
            return {"ok": False, "error": f"Monitor connection failed: {exc}"}
        except asyncio.TimeoutError:
            return {"ok": False, "error": "Monitor response timeout"}

    async def send_monitor_command(self, command: str) -> dict:
        """Send an arbitrary command to the QEMU monitor."""
        if self._proc is None or self._proc.returncode is not None:
            return {"ok": False, "error": "QEMU is not running"}

        try:
            reader, writer = await asyncio.open_unix_connection(self._monitor_sock_path)
            await asyncio.wait_for(reader.read(4096), timeout=1)

            writer.write(f"{command}\n".encode())
            await writer.drain()
            await asyncio.sleep(0.2)
            resp = await asyncio.wait_for(reader.read(8192), timeout=2)
            writer.close()
            await writer.wait_closed()
            return {"ok": True, "response": resp.decode(errors="replace").strip()}
        except (ConnectionRefusedError, FileNotFoundError) as exc:
            return {"ok": False, "error": f"Monitor connection failed: {exc}"}
        except asyncio.TimeoutError:
            return {"ok": False, "error": "Monitor response timeout"}
