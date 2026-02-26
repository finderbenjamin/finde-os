#!/bin/bash
set -euo pipefail

make clean
make

timeout 10s qemu-system-x86_64 \
  -cdrom build/os.iso \
  -serial stdio \
  -display none \
  -no-reboot \
  -no-shutdown | tee log.txt

grep -q "BOOT_OK" log.txt