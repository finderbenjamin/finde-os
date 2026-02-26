#!/bin/bash
set -euo pipefail

run_qemu() {
  local log_file="$1"

  rm -f "${log_file}"
  set +e
  timeout 10s qemu-system-x86_64 \
    -cdrom build/os.iso \
    -serial stdio \
    -display none \
    -no-reboot \
    -no-shutdown | tee "${log_file}"
  local qemu_status=${PIPESTATUS[0]}
  set -e

  if [[ ${qemu_status} -ne 0 && ${qemu_status} -ne 124 ]]; then
    echo "QEMU failed with exit code ${qemu_status}" >&2
    exit 1
  fi
}

echo "[1/3] normal boot check"
make clean
make
run_qemu log.txt

if ! tr -d '\r' < log.txt | grep -Fxq "BOOT_OK"; then
  echo "Expected serial marker BOOT_OK not found" >&2
  exit 1
fi

echo "[2/3] panic path check"
make clean
make PANIC_TEST=1
run_qemu panic_log.txt

if ! tr -d '\r' < panic_log.txt | grep -Fq "PANIC:"; then
  echo "Expected panic marker PANIC: not found" >&2
  exit 1
fi

echo "[3/3] idt exception check"
make clean
make IDT_TEST=1
run_qemu idt_log.txt

if ! tr -d '\r' < idt_log.txt | grep -Fxq "IDT_OK"; then
  echo "Expected serial marker IDT_OK not found" >&2
  exit 1
fi

if ! tr -d '\r' < idt_log.txt | grep -Fxq "EXC:3"; then
  echo "Expected serial marker EXC:3 not found" >&2
  exit 1
fi

echo "PASS"
