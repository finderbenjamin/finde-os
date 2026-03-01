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

echo "[1/25] normal boot check"
make clean
make
run_qemu log.txt

if ! tr -d '\r' < log.txt | grep -Fq "IDT_OK"; then
  echo "Expected serial marker IDT_OK not found" >&2
  exit 1
fi

echo "[2/25] boot marker check"
make clean
make BOOT_TEST=1
run_qemu boot_log.txt

if ! tr -d '\r' < boot_log.txt | grep -Fq "BOOT_OK"; then
  echo "Expected serial marker BOOT_OK not found" >&2
  exit 1
fi

echo "[3/25] panic path check"
make clean
make PANIC_TEST=1
run_qemu panic_log.txt

if ! tr -d '\r' < panic_log.txt | grep -Fq "PANIC:"; then
  echo "Expected panic marker PANIC: not found" >&2
  exit 1
fi

echo "[4/25] idt exception check"
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

echo "[5/25] timer interrupt check"
make clean
make TIMER_TEST=1
run_qemu timer_log.txt

if ! tr -d '\r' < timer_log.txt | grep -Fq "TICK_OK"; then
  echo "Expected serial marker TICK_OK not found" >&2
  exit 1
fi

echo "[6/25] heap allocator check"
make clean
make HEAP_TEST=1
run_qemu heap_log.txt

if ! tr -d '\r' < heap_log.txt | grep -Fq "HEAP_OK"; then
  echo "Expected serial marker HEAP_OK not found" >&2
  exit 1
fi

echo "[7/25] shell test check"
make clean
make SHELL_TEST=1
run_qemu shell_log.txt

if ! tr -d '\r' < shell_log.txt | grep -Fq "SHELL_OK"; then
  echo "Expected serial marker SHELL_OK not found" >&2
  exit 1
fi

echo "[8/25] keyboard decoder check"
make clean
make KEYBOARD_TEST=1
run_qemu keyboard_log.txt

if ! tr -d '\r' < keyboard_log.txt | grep -Fq "KBD_OK"; then
  echo "Expected serial marker KBD_OK not found" >&2
  exit 1
fi

echo "[9/25] virtual memory stack check"
make clean
make VM_TEST=1
run_qemu vm_log.txt

if ! tr -d '\r' < vm_log.txt | grep -Fq "VM_OK"; then
  echo "Expected serial marker VM_OK not found" >&2
  exit 1
fi

echo "[10/25] physical memory manager check"
make clean
make PMM_TEST=1
run_qemu pmm_log.txt

if ! tr -d '\r' < pmm_log.txt | grep -Fq "PMM_OK"; then
  echo "Expected serial marker PMM_OK not found" >&2
  exit 1
fi

echo "[11/25] virtual memory mapping check"
make clean
make VMM_TEST=1
run_qemu vmm_log.txt

if ! tr -d '\r' < vmm_log.txt | grep -Fq "VMM_OK"; then
  echo "Expected serial marker VMM_OK not found" >&2
  exit 1
fi

echo "[12/25] page fault handler check"
make clean
make PF_TEST=1
run_qemu pf_log.txt

if ! tr -d '\r' < pf_log.txt | grep -Fxq "PF_OK"; then
  echo "Expected serial marker PF_OK not found" >&2
  exit 1
fi

echo "[13/25] nx execute protection check"
make clean
make NX_TEST=1
run_qemu nx_log.txt

if ! tr -d '\r' < nx_log.txt | grep -Fxq "NX_OK"; then
  echo "Expected serial marker NX_OK not found" >&2
  exit 1
fi

echo "[14/25] VGA console check"
make clean
make VGA_TEST=1
run_qemu vga_log.txt

if ! tr -d '\r' < vga_log.txt | sed -E 's/\x1B\[[0-9;]*[[:alpha:]]//g' | grep -Fxq "VGA_OK"; then
  echo "Expected serial marker VGA_OK not found" >&2
  exit 1
fi

echo "[15/25] shell line editing check"
make clean
make EDIT_TEST=1
run_qemu edit_log.txt

if ! tr -d '\r' < edit_log.txt | grep -Fxq "EDIT_OK"; then
  echo "Expected serial marker EDIT_OK not found" >&2
  exit 1
fi

echo "[16/25] capability primitives check"
make clean
make CAP_TEST=1
run_qemu cap_log.txt

if ! tr -d '\r' < cap_log.txt | grep -Fq "CAP_OK"; then
  echo "Expected serial marker CAP_OK not found" >&2
  exit 1
fi

echo "[17/25] capability enforcement check"
make clean
make CAP_ENFORCE_TEST=1
run_qemu cap_enforce_log.txt

if ! tr -d '\r' < cap_enforce_log.txt | grep -Fq "CAP_ENFORCE_OK"; then
  echo "Expected serial marker CAP_ENFORCE_OK not found" >&2
  exit 1
fi


echo "[18/25] syscall capability gate check"
make clean
make SYSCALL_TEST=1
run_qemu syscall_log.txt

if ! tr -d '\r' < syscall_log.txt | grep -Fq "SYSCALL_OK"; then
  echo "Expected serial marker SYSCALL_OK not found" >&2
  exit 1
fi




echo "[19/25] syscall deny path check"
make clean
make SYSCALL_DENY_TEST=1
run_qemu syscall_deny_log.txt

if ! tr -d '\r' < syscall_deny_log.txt | grep -Fq "SYSCALL_DENY_OK"; then
  echo "Expected serial marker SYSCALL_DENY_OK not found" >&2
  exit 1
fi

echo "[20/25] task scheduler check"
make clean
make TASK_TEST=1
run_qemu task_log.txt

if ! tr -d '\r' < task_log.txt | grep -Fq "TASK_OK"; then
  echo "Expected serial marker TASK_OK not found" >&2
  exit 1
fi


echo "[21/25] task capability isolation check"
make clean
make TASK_CAP_TEST=1
run_qemu task_cap_log.txt

if ! tr -d '\r' < task_cap_log.txt | grep -Fq "TASK_CAP_OK"; then
  echo "Expected serial marker TASK_CAP_OK not found" >&2
  exit 1
fi


echo "[22/25] capability generation guard check"
make clean
make CAP_GEN_TEST=1
run_qemu cap_gen_log.txt

if ! tr -d '\r' < cap_gen_log.txt | grep -Fq "CAP_GEN_OK"; then
  echo "Expected serial marker CAP_GEN_OK not found" >&2
  exit 1
fi



echo "[23/25] user mode launch path check"
make clean
make USERMODE_TEST=1
run_qemu usermode_log.txt

if ! tr -d '\r' < usermode_log.txt | grep -Fq "USERMODE_OK"; then
  echo "Expected serial marker USERMODE_OK not found" >&2
  exit 1
fi


echo "[24/25] user task capability path check"
make clean
make USER_TASK_TEST=1
run_qemu user_task_log.txt

if ! tr -d '\r' < user_task_log.txt | grep -Fq "USER_TASK_OK"; then
  echo "Expected serial marker USER_TASK_OK not found" >&2
  exit 1
fi


echo "[25/25] user task capability deny path check"
make clean
make USER_TASK_DENY_TEST=1
run_qemu user_task_deny_log.txt

if ! tr -d '\r' < user_task_deny_log.txt | grep -Fq "USER_TASK_DENY_OK"; then
  echo "Expected serial marker USER_TASK_DENY_OK not found" >&2
  exit 1
fi

echo "PASS"
