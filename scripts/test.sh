#!/bin/bash
set -euo pipefail

make clean
make

rm -f log.txt
set +e
timeout 10s qemu-system-x86_64 \
  -cdrom build/os.iso \
  -serial stdio \
  -display none \
  -no-reboot \
  -no-shutdown | tee log.txt
qemu_status=${PIPESTATUS[0]}
set -e

if [[ ${qemu_status} -ne 0 && ${qemu_status} -ne 124 ]]; then
  echo "QEMU failed with exit code ${qemu_status}" >&2
  exit 1
fi

if ! tr -d '\r' < log.txt | grep -Fxq "BOOT_OK"; then
  echo "Expected serial marker BOOT_OK not found" >&2
  exit 1
fi

echo "PASS"
