#!/bin/bash
set -euo pipefail

run_qemu() {
  rm -f log.txt
  set +e
  timeout 10s qemu-system-x86_64 \
    -cdrom build/os.iso \
    -serial stdio \
    -display none \
    -no-reboot \
    -no-shutdown | tee log.txt
  local qemu_status=${PIPESTATUS[0]}
  set -e

  if [[ ${qemu_status} -ne 0 && ${qemu_status} -ne 124 ]]; then
    echo "QEMU failed with exit code ${qemu_status}" >&2
    exit 1
  fi
}

make clean
make

run_qemu

grep -q "BOOT_OK" log.txt
grep -q "SCROLL_OK" log.txt
