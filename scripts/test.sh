#!/bin/bash
set -e

make

timeout 10 qemu-system-x86_64 \
  -cdrom build/os.iso \
  -serial stdio \
  -display none | tee log.txt

grep -q "BOOT_OK" log.txt