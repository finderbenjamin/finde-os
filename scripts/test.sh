#!/bin/bash
set -euo pipefail

run_qemu() {
  local log_file="$1"

  rm -f "${log_file}"
  set +e
  timeout 10s qemu-system-x86_64 \
    -cdrom build/finde-os.iso \
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

echo "[1/9] normal boot check"
make clean
make
run_qemu log.txt

if ! tr -d '\r' < log.txt | grep -Fq "IDT_OK"; then
  echo "Expected serial marker IDT_OK not found" >&2
  exit 1
fi

echo "[2/9] boot marker check"
make clean
make BOOT_TEST=1
run_qemu boot_log.txt

if ! tr -d '\r' < boot_log.txt | grep -Fq "BOOT_OK"; then
  echo "Expected serial marker BOOT_OK not found" >&2
  exit 1
fi

echo "[3/9] panic path check"
make clean
make PANIC_TEST=1
run_qemu panic_log.txt

if ! tr -d '\r' < panic_log.txt | grep -Fq "PANIC:"; then
  echo "Expected panic marker PANIC: not found" >&2
  exit 1
fi

echo "[4/9] idt exception check"
make clean
make IDT_TEST=1
run_qemu idt_log.txt

if ! tr -d '\r' < idt_log.txt | grep -Fq "IDT_OK"; then
  echo "Expected serial marker IDT_OK not found" >&2
  exit 1
fi

if ! tr -d '\r' < idt_log.txt | grep -Fq "EXC:4"; then
  echo "Expected serial marker EXC:4 not found" >&2
  exit 1
fi

echo "[5/9] timer interrupt check"
make clean
make TIMER_TEST=1
run_qemu timer_log.txt

if ! tr -d '\r' < timer_log.txt | grep -Fq "TICK_OK"; then
  echo "Expected serial marker TICK_OK not found" >&2
  exit 1
fi

echo "[6/9] heap allocator check"
make clean
make HEAP_TEST=1
run_qemu heap_log.txt

if ! tr -d '\r' < heap_log.txt | grep -Fq "HEAP_OK"; then
  echo "Expected serial marker HEAP_OK not found" >&2
  exit 1
fi

echo "[7/9] shell test check"
make clean
make SHELL_TEST=1
run_qemu shell_log.txt

if ! tr -d '\r' < shell_log.txt | grep -Fq "SHELL_OK"; then
  echo "Expected serial marker SHELL_OK not found" >&2
  exit 1
fi

echo "[8/9] keyboard decoder check"
make clean
make KEYBOARD_TEST=1
run_qemu keyboard_log.txt

if ! tr -d '\r' < keyboard_log.txt | grep -Fq "KBD_OK"; then
  echo "Expected serial marker KBD_OK not found" >&2
  exit 1
fi

echo "[9/9] virtual memory stack check"
make clean
make VM_TEST=1
run_qemu vm_log.txt

if ! tr -d '\r' < vm_log.txt | grep -Fq "VM_OK"; then
  echo "Expected serial marker VM_OK not found" >&2
  exit 1
fi

echo "PASS"
