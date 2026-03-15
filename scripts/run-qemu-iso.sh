#!/usr/bin/env bash
set -euo pipefail

print_usage() {
  cat <<USAGE
Usage: $(basename "$0") [--iso PATH] [--gdb] [ISO]
Defaults: ISO=grub2/os.iso. --gdb enables gdb stub (-s -S).
Example: $(basename "$0") --iso grub2/os.iso --gdb
USAGE
}

ISO="grub2/os.iso"
GDB=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --iso) ISO="$2"; shift 2;;
    --gdb) GDB=1; shift;;
    -h|--help) print_usage; exit 0;;
    *) ISO="$1"; shift;;
  esac
done

if [ ! -f "$ISO" ]; then
  echo "Error: ISO not found at $ISO" >&2
  exit 2
fi

CMD=(qemu-system-i386 -cdrom "$ISO" -serial stdio -display none)
if [ "$GDB" -eq 1 ]; then CMD+=(-s -S); fi

echo "Launching: ${CMD[*]}"
exec "${CMD[@]}"
