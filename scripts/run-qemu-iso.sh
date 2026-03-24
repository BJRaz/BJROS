#!/usr/bin/env bash
set -euo pipefail

print_usage() {
  cat <<USAGE
Usage: $(basename "$0") [--iso PATH] [--gdb] [--gui] [ISO]
Defaults: ISO=grub2/os.iso. Runs headless with serial console.
  --gdb   Enable GDB stub (-s -S)
  --gui   Launch with VGA display window instead of headless
Example: $(basename "$0") --iso grub2/os.iso --gdb
USAGE
}

ISO="grub2/os.iso"
GDB=0
GUI=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --iso) ISO="$2"; shift 2;;
    --gdb) GDB=1; shift;;
    --gui) GUI=1; shift;;
    -h|--help) print_usage; exit 0;;
    *) ISO="$1"; shift;;
  esac
done

if [ ! -f "$ISO" ]; then
  echo "Error: ISO not found at $ISO" >&2
  exit 2
fi

if [ "$GUI" -eq 1 ]; then
  CMD=(qemu-system-i386 -cdrom "$ISO")
else
  CMD=(qemu-system-i386 -cdrom "$ISO" -nographic)
fi
if [ "$GDB" -eq 1 ]; then CMD+=(-s -S); fi

echo "Launching: ${CMD[*]}"
exec "${CMD[@]}"
