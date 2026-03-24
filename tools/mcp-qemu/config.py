"""Default configuration for the MCP-QEMU server."""

import os
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent

# Kernel artifacts
KERNEL_ELF = os.environ.get(
    "BJROS_KERNEL_ELF",
    str(REPO_ROOT / "build" / "x86" / "kernel.elf"),
)
KERNEL_ISO = os.environ.get(
    "BJROS_KERNEL_ISO",
    str(REPO_ROOT / "grub2" / "os.iso"),
)

# QEMU
QEMU_BIN = os.environ.get("QEMU_BIN", "qemu-system-i386")
QEMU_MEMORY = os.environ.get("QEMU_MEMORY", "128M")
QEMU_GDB_PORT = int(os.environ.get("QEMU_GDB_PORT", "1234"))
QEMU_MONITOR_SOCK = os.environ.get(
    "QEMU_MONITOR_SOCK",
    "/tmp/bjros-qemu-monitor.sock",
)

# GDB
GDB_BIN = os.environ.get("GDB_BIN", "gdb")
GDB_CONNECT_TIMEOUT = int(os.environ.get("GDB_CONNECT_TIMEOUT", "10"))

# Serial buffer
SERIAL_BUFFER_MAX = 64 * 1024  # 64 KiB ring buffer
