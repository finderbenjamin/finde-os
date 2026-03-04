#!/bin/bash
set -euo pipefail

run_qemu() {
  local log_file="$1"

  rm -f "${log_file}"
  set +e
  timeout 20s qemu-system-x86_64 \
    -cdrom build/finde-os.iso \
    -serial stdio \
    -display none \
    -no-reboot \
    -no-shutdown 2>&1 | tee "${log_file}"
  local qemu_status=${PIPESTATUS[0]}
  set -e

  if [[ ${qemu_status} -ne 0 && ${qemu_status} -ne 124 ]]; then
    echo "QEMU failed with exit code ${qemu_status}" >&2
    exit 1
  fi
}

echo "[1/40] normal boot check"
make clean
make
run_qemu log.txt

if ! tr -d '\r' < log.txt | grep -Fq "IDT_OK"; then
  echo "Expected serial marker IDT_OK not found" >&2
  exit 1
fi

echo "[2/40] boot marker check"
make clean
make BOOT_TEST=1
run_qemu boot_log.txt

if ! tr -d '\r' < boot_log.txt | grep -Fq "BOOT_OK"; then
  echo "Expected serial marker BOOT_OK not found" >&2
  exit 1
fi

echo "[3/40] panic path check"
make clean
make PANIC_TEST=1
run_qemu panic_log.txt

if ! tr -d '\r' < panic_log.txt | grep -Fq "PANIC:"; then
  echo "Expected panic marker PANIC: not found" >&2
  exit 1
fi

echo "[4/40] idt exception check"
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

echo "[5/40] timer interrupt check"
make clean
make TIMER_TEST=1
run_qemu timer_log.txt

if ! tr -d '\r' < timer_log.txt | grep -Fq "TICK_OK"; then
  echo "Expected serial marker TICK_OK not found" >&2
  exit 1
fi

echo "[6/40] heap allocator check"
make clean
make HEAP_TEST=1
run_qemu heap_log.txt

if ! tr -d '\r' < heap_log.txt | grep -Fq "HEAP_OK"; then
  echo "Expected serial marker HEAP_OK not found" >&2
  exit 1
fi

echo "[7/40] shell test check"
make clean
make SHELL_TEST=1
run_qemu shell_log.txt

if ! tr -d '\r' < shell_log.txt | grep -Fq "SHELL_OK"; then
  echo "Expected serial marker SHELL_OK not found" >&2
  exit 1
fi

echo "[8/40] keyboard decoder check"
make clean
make KEYBOARD_TEST=1
run_qemu keyboard_log.txt

if ! tr -d '\r' < keyboard_log.txt | grep -Fq "KBD_OK"; then
  echo "Expected serial marker KBD_OK not found" >&2
  exit 1
fi

echo "[9/40] virtual memory stack check"
make clean
make VM_TEST=1
run_qemu vm_log.txt

if ! tr -d '\r' < vm_log.txt | grep -Fq "VM_OK"; then
  echo "Expected serial marker VM_OK not found" >&2
  exit 1
fi

echo "[10/40] physical memory manager check"
make clean
make PMM_TEST=1
run_qemu pmm_log.txt

if ! tr -d '\r' < pmm_log.txt | grep -Fq "PMM_OK"; then
  echo "Expected serial marker PMM_OK not found" >&2
  exit 1
fi

echo "[11/40] virtual memory mapping check"
make clean
make VMM_TEST=1
run_qemu vmm_log.txt

if ! tr -d '\r' < vmm_log.txt | grep -Fq "VMM_OK"; then
  echo "Expected serial marker VMM_OK not found" >&2
  exit 1
fi

echo "[12/40] page fault handler check"
make clean
make PF_TEST=1
run_qemu pf_log.txt

if ! tr -d '\r' < pf_log.txt | grep -Fxq "PF_OK"; then
  echo "Expected serial marker PF_OK not found" >&2
  exit 1
fi

echo "[13/40] nx execute protection check"
make clean
make NX_TEST=1
run_qemu nx_log.txt

if ! tr -d '\r' < nx_log.txt | grep -Fxq "NX_OK"; then
  echo "Expected serial marker NX_OK not found" >&2
  exit 1
fi

echo "[14/40] VGA console check"
make clean
make VGA_TEST=1
run_qemu vga_log.txt

if ! tr -d '\r' < vga_log.txt | sed -E 's/\x1B\[[0-9;]*[[:alpha:]]//g' | grep -Fxq "VGA_OK"; then
  echo "Expected serial marker VGA_OK not found" >&2
  exit 1
fi

echo "[15/40] shell line editing check"
make clean
make EDIT_TEST=1
run_qemu edit_log.txt

if ! tr -d '\r' < edit_log.txt | grep -Fxq "EDIT_OK"; then
  echo "Expected serial marker EDIT_OK not found" >&2
  exit 1
fi

echo "[16/40] capability primitives check"
make clean
make CAP_TEST=1
run_qemu cap_log.txt

if ! tr -d '\r' < cap_log.txt | grep -Fq "CAP_OK"; then
  echo "Expected serial marker CAP_OK not found" >&2
  exit 1
fi

echo "[17/40] capability enforcement check"
make clean
make CAP_ENFORCE_TEST=1
run_qemu cap_enforce_log.txt

if ! tr -d '\r' < cap_enforce_log.txt | grep -Fq "CAP_ENFORCE_OK"; then
  echo "Expected serial marker CAP_ENFORCE_OK not found" >&2
  exit 1
fi


echo "[18/40] syscall capability gate check"
make clean
make SYSCALL_TEST=1
run_qemu syscall_log.txt

if ! tr -d '\r' < syscall_log.txt | grep -Fq "SYSCALL_OK"; then
  echo "Expected serial marker SYSCALL_OK not found" >&2
  exit 1
fi




echo "[19/40] syscall deny path check"
make clean
make SYSCALL_DENY_TEST=1
run_qemu syscall_deny_log.txt

if ! tr -d '\r' < syscall_deny_log.txt | grep -Fq "SYSCALL_DENY_OK"; then
  echo "Expected serial marker SYSCALL_DENY_OK not found" >&2
  exit 1
fi

echo "[20/40] task scheduler check"
make clean
make TASK_TEST=1
run_qemu task_log.txt

if ! tr -d '\r' < task_log.txt | grep -Fq "TASK_OK"; then
  echo "Expected serial marker TASK_OK not found" >&2
  exit 1
fi


echo "[21/40] task capability isolation check"
make clean
make TASK_CAP_TEST=1
run_qemu task_cap_log.txt

if ! tr -d '\r' < task_cap_log.txt | grep -Fq "TASK_CAP_OK"; then
  echo "Expected serial marker TASK_CAP_OK not found" >&2
  exit 1
fi


echo "[22/40] capability generation guard check"
make clean
make CAP_GEN_TEST=1
run_qemu cap_gen_log.txt

if ! tr -d '\r' < cap_gen_log.txt | grep -Fq "CAP_GEN_OK"; then
  echo "Expected serial marker CAP_GEN_OK not found" >&2
  exit 1
fi



echo "[23/40] user mode launch path check"
make clean
make USERMODE_TEST=1
run_qemu usermode_log.txt

if ! tr -d '\r' < usermode_log.txt | grep -Fq "USERMODE_OK"; then
  echo "Expected serial marker USERMODE_OK not found" >&2
  exit 1
fi


echo "[24/40] user task capability path check"
make clean
make USER_TASK_TEST=1
run_qemu user_task_log.txt

if ! tr -d '\r' < user_task_log.txt | grep -Fq "USER_TASK_OK"; then
  echo "Expected serial marker USER_TASK_OK not found" >&2
  exit 1
fi


echo "[25/40] user task capability deny path check"
make clean
make USER_TASK_DENY_TEST=1
run_qemu user_task_deny_log.txt

if ! tr -d '\r' < user_task_deny_log.txt | grep -Fq "USER_TASK_DENY_OK"; then
  echo "Expected serial marker USER_TASK_DENY_OK not found" >&2
  exit 1
fi

echo "[26/40] user-space driver isolation check"
make clean
make DRV_ISO_TEST=1
run_qemu drv_iso_log.txt

if ! tr -d '\r' < drv_iso_log.txt | grep -Fq "DRV_ISO_OK"; then
  echo "Expected serial marker DRV_ISO_OK not found" >&2
  exit 1
fi



echo "[27/40] microvm isolation smoke check"
make clean
make MICROVM_TEST=1
run_qemu microvm_log.txt

if ! tr -d '\r' < microvm_log.txt | grep -Fq "MICROVM_OK"; then
  echo "Expected serial marker MICROVM_OK not found" >&2
  exit 1
fi


echo "[28/40] microvm capability boundary check"
make clean
make MICROVM_CAP_TEST=1
run_qemu microvm_cap_log.txt

if ! tr -d '\r' < microvm_cap_log.txt | grep -Fq "MICROVM_CAP_OK"; then
  echo "Expected serial marker MICROVM_CAP_OK not found" >&2
  exit 1
fi


echo "[29/40] inter-vm capability isolation check"
make clean
make INTERVM_TEST=1
run_qemu intervm_log.txt

if ! tr -d '\r' < intervm_log.txt | grep -Fq "INTERVM_OK"; then
  echo "Expected serial marker INTERVM_OK not found" >&2
  exit 1
fi


echo "[30/40] ipc capability channel check"
make clean
make IPC_TEST=1
run_qemu ipc_log.txt

if ! tr -d '\r' < ipc_log.txt | grep -Fq "IPC_OK"; then
  echo "Expected serial marker IPC_OK not found" >&2
  exit 1
fi


echo "[31/40] capability revocation propagation check"
make clean
make REVOKE_TEST=1
run_qemu revoke_log.txt

if ! tr -d '\r' < revoke_log.txt | grep -Fq "REVOKE_OK"; then
  echo "Expected serial marker REVOKE_OK not found" >&2
  exit 1
fi


echo "[32/40] syscall rate guard check"
make clean
make DOS_GUARD_TEST=1
run_qemu dos_guard_log.txt

if ! tr -d '\r' < dos_guard_log.txt | grep -Fq "DOS_GUARD_OK"; then
  echo "Expected serial marker DOS_GUARD_OK not found" >&2
  exit 1
fi


echo "[33/40] process/task resource quota check"
make clean
make QUOTA_TEST=1
run_qemu quota_log.txt

if ! tr -d '\r' < quota_log.txt | grep -Fq "QUOTA_OK"; then
  echo "Expected serial marker QUOTA_OK not found" >&2
  exit 1
fi


echo "[34/40] capability type isolation matrix check"
make clean
make CAP_TYPE_TEST=1
run_qemu cap_type_log.txt

if ! tr -d '\r' < cap_type_log.txt | grep -Fq "CAP_TYPE_OK"; then
  echo "Expected serial marker CAP_TYPE_OK not found" >&2
  exit 1
fi

echo "[35/40] capability lifecycle check"
make clean
make CAP_LIFECYCLE_TEST=1
run_qemu cap_lifecycle_log.txt

if ! tr -d '\r' < cap_lifecycle_log.txt | grep -Fq "CAP_LIFECYCLE_OK"; then
  echo "Expected serial marker CAP_LIFECYCLE_OK not found" >&2
  exit 1
fi

echo "[36/40] ipc + inter-vm platform hardening check"
make clean
make IPC_PLATFORM_TEST=1
run_qemu ipc_platform_log.txt

if ! tr -d '\r' < ipc_platform_log.txt | grep -Fq "IPC_PLATFORM_OK"; then
  echo "Expected serial marker IPC_PLATFORM_OK not found" >&2
  exit 1
fi

echo "[37/40] limits telemetry + deny reason check"
make clean
make LIMITS_TEST=1
run_qemu limits_log.txt

if ! tr -d '\r' < limits_log.txt | grep -Fq "LIMITS_OK"; then
  echo "Expected serial marker LIMITS_OK not found" >&2
  exit 1
fi

echo "[38/40] usermode + driver path hardening check"
make clean
make USERMODE_PATH_TEST=1
run_qemu usermode_path_log.txt

if ! tr -d '\r' < usermode_path_log.txt | grep -Fq "USERMODE_PATH_OK"; then
  echo "Expected serial marker USERMODE_PATH_OK not found" >&2
  exit 1
fi

echo "[39/40] microvm golden security mode boundary check"
make clean
make MICROVM_MODE_TEST=1
run_qemu microvm_mode_log.txt

if ! tr -d '\r' < microvm_mode_log.txt | grep -Fq "MICROVM_MODE_OK"; then
  echo "Expected serial marker MICROVM_MODE_OK not found" >&2
  exit 1
fi

echo "[40/40] unified mode manager launch path check"
make clean
make MODE_MANAGER_TEST=1
run_qemu mode_manager_log.txt

if ! tr -d '\r' < mode_manager_log.txt | grep -Fq "MODE_MANAGER_OK"; then
  echo "Expected serial marker MODE_MANAGER_OK not found" >&2
  exit 1
fi

echo "[41/42] CLI security introspection check"
make clean
make CLI_SECURITY_TEST=1
run_qemu cli_security_log.txt

if ! tr -d '\r' < cli_security_log.txt | grep -Fq "CLI_SECURITY_OK"; then
  echo "Expected serial marker CLI_SECURITY_OK not found" >&2
  exit 1
fi

if ! tr -d '\r' < cli_security_log.txt | grep -Fq "CLI_SECURITY_MARKER:DENY=DENY: capability security check failed"; then
  echo "Expected deterministic deny marker not found" >&2
  exit 1
fi


echo "[42/42] CLI layered pipeline check"
make clean
make CLI_LAYERS_TEST=1
run_qemu cli_layers_log.txt

if ! tr -d '' < cli_layers_log.txt | grep -Fq "CLI_LAYERS_OK"; then
  echo "Expected serial marker CLI_LAYERS_OK not found" >&2
  exit 1
fi

echo "PASS"
