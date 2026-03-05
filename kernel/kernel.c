#include <stddef.h>
#include <stdint.h>

#include "console.h"
#include "cap.h"
#include "cli_executor.h"
#include "cli_parser.h"
#include "cli_validator.h"
#include "heap.h"
#include "idt.h"
#include "keyboard.h"
#include "multiboot1.h"
#include "multiboot2.h"
#include "paging.h"
#include "panic.h"
#include "pmm.h"
#include "serial.h"
#include "shell.h"
#include "vga.h"

extern void isr_4(void);
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

volatile uint8_t g_nx_expect = 0;

#if defined(PMM_TEST) || defined(VMM_TEST) || defined(NX_TEST)
static uint64_t align_down_4k(uint64_t value) {
  return value & ~0xFFFull;
}

static uint64_t align_up_4k(uint64_t value) {
  return (value + 0xFFFull) & ~0xFFFull;
}
#endif








#ifdef PMM_TEST
static __attribute__((noreturn)) void pmm_test_fail(void) {
  serial_write("PMM_FAIL\n");
  panic("PMM_TEST");
}

static __attribute__((noreturn)) void pmm_test_halt_success(void) {
  serial_write("PMM_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif



#ifdef PF_TEST
static __attribute__((noreturn)) void pf_test_fail(void) {
  serial_write("PF_FAIL\n");
  panic("PF_TEST");
}
#endif

#ifdef VMM_TEST
static __attribute__((noreturn)) void vmm_test_fail(void) {
  serial_write("VMM_FAIL\n");
  panic("VMM_TEST");
}

static __attribute__((noreturn)) void vmm_test_halt_success(void) {
  serial_write("VMM_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif


#ifdef NX_TEST
static __attribute__((noreturn)) void nx_test_fail(void) {
  serial_write("NX_FAIL\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif

#ifdef EDIT_TEST
static int edit_line_equals(const char* a, const char* b) {
  while (*a != '\0' && *b != '\0') {
    if (*a != *b) {
      return 0;
    }
    ++a;
    ++b;
  }
  return *a == '\0' && *b == '\0';
}

static __attribute__((noreturn)) void edit_test_halt(void) {
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif

#ifdef HEAP_TEST
static void heap_test(void) {
  static const size_t sizes[] = {1, 7, 16, 31, 64, 129, 1024, 4096};
  uint8_t* blocks[sizeof(sizes) / sizeof(sizes[0])];

  heap_init();

  for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
    blocks[i] = (uint8_t*)kmalloc(sizes[i]);
    if (blocks[i] == (void*)0) {
      panic("HEAP_TEST OOM");
    }
  }
}
#endif

#ifdef KEYBOARD_TEST
static void keyboard_test(void) {
  if (keyboard_decode_scancode_for_test(0x1E, 0) != 'a') {
    panic("KBD_BAD_A");
  }
  if (keyboard_decode_scancode_for_test(0x10, 1) != 'Q') {
    panic("KBD_BAD_Q_UP");
  }
  if (keyboard_decode_scancode_for_test(0x39, 0) != ' ') {
    panic("KBD_BAD_SPACE");
  }
  if (keyboard_decode_scancode_for_test(0x1C, 0) != '\n') {
    panic("KBD_BAD_NL");
  }
  if (keyboard_decode_scancode_for_test(0x0E, 0) != '\b') {
    panic("KBD_BAD_BS");
  }
}
#endif





#ifdef CAP_ENFORCE_TEST
static __attribute__((noreturn)) void cap_enforce_test_fail(void) {
  serial_write("CAP_ENFORCE_FAIL\n");
  panic("CAP_ENFORCE_TEST");
}

static __attribute__((noreturn)) void cap_enforce_test_halt_success(void) {
  serial_write("CAP_ENFORCE_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

static int cap_enforce_require_write(uint64_t handle) {
  return cap_check(handle, 1u, CAP_R_WRITE);
}
#endif



#if defined(SYSCALL_DENY_TEST) || defined(CLI_SECURITY_TEST)
static __attribute__((noreturn)) void syscall_deny_test_fail(void) {
  serial_write("SYSCALL_DENY_FAIL\n");
  panic("SYSCALL_DENY_TEST");
}

static __attribute__((noreturn)) void syscall_deny_test_halt_success(void) {
  serial_write("SYSCALL_DENY_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

static int syscall_deny_log_write(uint64_t handle, uint32_t pid, const char* msg) {
  if (cap_require(handle, pid, CAP_R_WRITE, 0) != CAP_REQUIRE_OK) {
    return 0;
  }

  serial_write(msg);
  return 1;
}
#endif

#if defined(SYSCALL_TEST) || defined(CLI_SECURITY_TEST)
static __attribute__((noreturn)) void syscall_test_fail(void) {
  serial_write("SYSCALL_FAIL\n");
  panic("SYSCALL_TEST");
}

static __attribute__((noreturn)) void syscall_test_halt_success(void) {
  serial_write("SYSCALL_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

static int syscall_log_write(uint64_t handle, uint32_t pid, const char* msg) {
  if (cap_require(handle, pid, CAP_R_WRITE, 0) != CAP_REQUIRE_OK) {
    return 0;
  }

  serial_write(msg);
  return 1;
}
#endif


#ifdef TASK_TEST
static __attribute__((noreturn)) void task_test_fail(void) {
  serial_write("TASK_FAIL\n");
  panic("TASK_TEST");
}

static __attribute__((noreturn)) void task_test_halt_success(void) {
  serial_write("TASK_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

typedef struct {
  uint64_t id;
  uint64_t budget;
  uint64_t runs;
} task_test_task_t;

static int task_test_run(task_test_task_t* task) {
  if (task->runs >= task->budget) {
    return 0;
  }

  task->runs += 1;
  return 1;
}
#endif

#ifdef TASK_CAP_TEST
static __attribute__((noreturn)) void task_cap_test_fail(void) {
  serial_write("TASK_CAP_FAIL\n");
  panic("TASK_CAP_TEST");
}

static __attribute__((noreturn)) void task_cap_test_halt_success(void) {
  serial_write("TASK_CAP_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

typedef struct {
  uint32_t pid;
  uint64_t write_cap;
  uint64_t read_cap;
} task_cap_test_task_t;

static int task_cap_test_sys_write(const task_cap_test_task_t* task) {
  if (cap_check(task->write_cap, task->pid, CAP_R_WRITE) == 0) {
    return 0;
  }

  serial_write("TASK_CAP_PATH\n");
  return 1;
}
#endif

#ifdef CAP_GEN_TEST
static __attribute__((noreturn)) void cap_gen_test_fail(void) {
  serial_write("CAP_GEN_FAIL\n");
  panic("CAP_GEN_TEST");
}

static __attribute__((noreturn)) void cap_gen_test_halt_success(void) {
  serial_write("CAP_GEN_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif


#ifdef USERMODE_TEST
static __attribute__((noreturn)) void usermode_test_fail(void) {
  serial_write("USERMODE_FAIL\n");
  panic("USERMODE_TEST");
}

static __attribute__((noreturn)) void usermode_test_halt_success(void) {
  serial_write("USERMODE_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif


#ifdef USER_TASK_TEST
static __attribute__((noreturn)) void user_task_test_fail(void) {
  serial_write("USER_TASK_FAIL\n");
  panic("USER_TASK_TEST");
}

static __attribute__((noreturn)) void user_task_test_halt_success(void) {
  serial_write("USER_TASK_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

static int user_task_sys_write(uint64_t handle, uint32_t pid, const char* msg) {
  if (cap_require(handle, pid, CAP_R_WRITE, 0) != CAP_REQUIRE_OK) {
    return 0;
  }

  serial_write(msg);
  return 1;
}
#endif

#if defined(USER_TASK_DENY_TEST) || defined(CLI_SECURITY_TEST)
static __attribute__((noreturn)) void user_task_deny_test_fail(void) {
  serial_write("USER_TASK_DENY_FAIL\n");
  panic("USER_TASK_DENY_TEST");
}

static __attribute__((noreturn)) void user_task_deny_test_halt_success(void) {
  serial_write("USER_TASK_DENY_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

static int user_task_deny_sys_write(uint64_t handle, uint32_t pid, const char* msg) {
  if (cap_require(handle, pid, CAP_R_WRITE, 0) != CAP_REQUIRE_OK) {
    return 0;
  }

  serial_write(msg);
  return 1;
}
#endif


#ifdef DRV_ISO_TEST
static __attribute__((noreturn)) void drv_iso_test_fail(void) {
  serial_write("DRV_ISO_FAIL\n");
  panic("DRV_ISO_TEST");
}

static __attribute__((noreturn)) void drv_iso_test_halt_success(void) {
  serial_write("DRV_ISO_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

static int drv_iso_mmio_write(uint64_t handle, uint32_t pid) {
  if (cap_check(handle, pid, CAP_R_WRITE) == 0) {
    return 0;
  }

  return 1;
}
#endif





#ifdef USERMODE_PATH_TEST
static __attribute__((noreturn)) void usermode_path_test_fail(void) {
  serial_write("USERMODE_PATH_FAIL\n");
  panic("USERMODE_PATH_TEST");
}

static __attribute__((noreturn)) void usermode_path_test_halt_success(void) {
  serial_write("USERMODE_PATH_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

enum {
  USERMODE_EXIT_OK = 0u,
  USERMODE_EXIT_DENIED = 1u,
  USERMODE_EXIT_STALE_CAP = 2u,
  USERMODE_EXIT_CRASH_ISOLATED = 3u,
};

typedef struct {
  uint32_t pid;
  uint64_t write_cap;
  uint32_t crashed;
} usermode_path_proc_t;

typedef struct {
  uint32_t last_exit_reason;
  uint32_t deny_count;
} usermode_path_telemetry_t;

static int usermode_path_string_eq(const char* a, const char* b) {
  while (*a != '\0' && *b != '\0') {
    if (*a != *b) {
      return 0;
    }
    ++a;
    ++b;
  }
  return *a == '\0' && *b == '\0';
}

static const char* usermode_path_exit_reason_text(uint32_t reason) {
  if (reason == USERMODE_EXIT_OK) {
    return "EXIT_OK";
  }
  if (reason == USERMODE_EXIT_DENIED) {
    return "EXIT_DENIED_CAPABILITY";
  }
  if (reason == USERMODE_EXIT_STALE_CAP) {
    return "EXIT_STALE_CAPABILITY";
  }
  if (reason == USERMODE_EXIT_CRASH_ISOLATED) {
    return "EXIT_CRASH_ISOLATED";
  }
  return "EXIT_UNKNOWN";
}

static int usermode_path_try_write(const usermode_path_proc_t* proc, uint32_t caller_pid,
                                   usermode_path_telemetry_t* telemetry) {
  if (cap_check(proc->write_cap, caller_pid, CAP_R_WRITE) == 0) {
    telemetry->deny_count += 1u;
    telemetry->last_exit_reason = USERMODE_EXIT_DENIED;
    return 0;
  }

  telemetry->last_exit_reason = USERMODE_EXIT_OK;
  return 1;
}

static int usermode_path_try_after_revoke(const usermode_path_proc_t* proc,
                                          usermode_path_telemetry_t* telemetry) {
  if (cap_check(proc->write_cap, proc->pid, CAP_R_WRITE) == 0) {
    telemetry->deny_count += 1u;
    telemetry->last_exit_reason = USERMODE_EXIT_STALE_CAP;
    return 0;
  }

  telemetry->last_exit_reason = USERMODE_EXIT_OK;
  return 1;
}

static void usermode_path_mark_crash(usermode_path_proc_t* proc,
                                     usermode_path_telemetry_t* telemetry) {
  proc->crashed = 1u;
  telemetry->last_exit_reason = USERMODE_EXIT_CRASH_ISOLATED;
}
#endif

#ifdef MICROVM_TEST
static __attribute__((noreturn)) void microvm_test_fail(void) {
  serial_write("MICROVM_FAIL\n");
  panic("MICROVM_TEST");
}

static __attribute__((noreturn)) void microvm_test_halt_success(void) {
  serial_write("MICROVM_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

typedef struct {
  uint32_t vmid;
  uint64_t io_cap;
} microvm_test_vm_t;

static int microvm_test_io_write(const microvm_test_vm_t* vm, uint32_t caller_vmid) {
  if (cap_check(vm->io_cap, caller_vmid, CAP_R_WRITE) == 0) {
    return 0;
  }

  return 1;
}
#endif

#ifdef MICROVM_CAP_TEST
static __attribute__((noreturn)) void microvm_cap_test_fail(void) {
  serial_write("MICROVM_CAP_FAIL\n");
  panic("MICROVM_CAP_TEST");
}

static __attribute__((noreturn)) void microvm_cap_test_halt_success(void) {
  serial_write("MICROVM_CAP_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

typedef struct {
  uint32_t vmid;
  uint64_t write_cap;
} microvm_cap_test_vm_t;

static int microvm_cap_test_write(const microvm_cap_test_vm_t* target, uint32_t caller_vmid) {
  if (cap_check(target->write_cap, caller_vmid, CAP_R_WRITE) == 0) {
    return 0;
  }

  return 1;
}
#endif




#ifdef MICROVM_MODE_TEST
static __attribute__((noreturn)) void microvm_mode_test_fail(void) {
  serial_write("MICROVM_MODE_FAIL\n");
  panic("MICROVM_MODE_TEST");
}

static __attribute__((noreturn)) void microvm_mode_test_halt_success(void) {
  serial_write("MICROVM_MODE_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

enum {
  MICROVM_MODE_SANDBOX = 0u,
  MICROVM_MODE_FULL = 1u,
};

enum {
  MICROVM_MODE_EXIT_OK = 0u,
  MICROVM_MODE_EXIT_SWITCH_DENIED = 1u,
  MICROVM_MODE_EXIT_BOUNDARY_DENIED = 2u,
  MICROVM_MODE_EXIT_STALE_CAP = 3u,
};

typedef struct {
  uint32_t pid;
  uint64_t sandbox_cap;
  uint64_t microvm_cap;
  uint32_t mode;
} microvm_mode_proc_t;

typedef struct {
  uint32_t last_mode;
  uint32_t last_exit_reason;
  uint32_t deny_count;
  uint32_t switch_count;
} microvm_mode_telemetry_t;

static int microvm_mode_string_eq(const char* a, const char* b) {
  while (*a != '\0' && *b != '\0') {
    if (*a != *b) {
      return 0;
    }
    ++a;
    ++b;
  }
  return *a == '\0' && *b == '\0';
}

static const char* microvm_mode_exit_reason_text(uint32_t reason) {
  if (reason == MICROVM_MODE_EXIT_OK) {
    return "MODE_OK";
  }
  if (reason == MICROVM_MODE_EXIT_SWITCH_DENIED) {
    return "MODE_SWITCH_DENIED";
  }
  if (reason == MICROVM_MODE_EXIT_BOUNDARY_DENIED) {
    return "MODE_BOUNDARY_DENIED";
  }
  if (reason == MICROVM_MODE_EXIT_STALE_CAP) {
    return "MODE_STALE_CAP";
  }
  return "MODE_UNKNOWN";
}

static int microvm_mode_switch(microvm_mode_proc_t* proc, uint32_t target_mode,
                               microvm_mode_telemetry_t* telemetry) {
  if (target_mode == MICROVM_MODE_FULL && cap_check(proc->microvm_cap, proc->pid, CAP_R_WRITE) == 0) {
    telemetry->deny_count += 1u;
    telemetry->last_exit_reason = MICROVM_MODE_EXIT_SWITCH_DENIED;
    return 0;
  }

  proc->mode = target_mode;
  telemetry->switch_count += 1u;
  telemetry->last_mode = target_mode;
  telemetry->last_exit_reason = MICROVM_MODE_EXIT_OK;
  return 1;
}

static int microvm_mode_run(const microvm_mode_proc_t* proc, uint32_t caller_pid,
                            microvm_mode_telemetry_t* telemetry) {
  if (proc->mode == MICROVM_MODE_SANDBOX) {
    if (cap_check(proc->sandbox_cap, caller_pid, CAP_R_READ) == 0) {
      telemetry->deny_count += 1u;
      telemetry->last_exit_reason = MICROVM_MODE_EXIT_BOUNDARY_DENIED;
      return 0;
    }

    telemetry->last_mode = MICROVM_MODE_SANDBOX;
    telemetry->last_exit_reason = MICROVM_MODE_EXIT_OK;
    return 1;
  }

  if (cap_check(proc->microvm_cap, proc->pid, CAP_R_WRITE) == 0) {
    telemetry->deny_count += 1u;
    telemetry->last_exit_reason = MICROVM_MODE_EXIT_STALE_CAP;
    return 0;
  }

  if (cap_check(proc->microvm_cap, caller_pid, CAP_R_WRITE) == 0) {
    telemetry->deny_count += 1u;
    telemetry->last_exit_reason = MICROVM_MODE_EXIT_BOUNDARY_DENIED;
    return 0;
  }

  telemetry->last_mode = MICROVM_MODE_FULL;
  telemetry->last_exit_reason = MICROVM_MODE_EXIT_OK;
  return 1;
}
#endif




#ifdef MODE_MANAGER_TEST
static __attribute__((noreturn)) void mode_manager_test_fail(void) {
  serial_write("MODE_MANAGER_FAIL\n");
  panic("MODE_MANAGER_TEST");
}

static __attribute__((noreturn)) void mode_manager_test_halt_success(void) {
  serial_write("MODE_MANAGER_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

enum {
  APP_POLICY_NORMAL = 0u,
  APP_POLICY_HIGH_ISOLATION = 1u,
};

enum {
  APP_MODE_SANDBOX = 0u,
  APP_MODE_MICROVM = 1u,
};

enum {
  MODE_EXIT_OK = 0u,
  MODE_EXIT_DENIED_POLICY = 1u,
  MODE_EXIT_DENIED_BOUNDARY = 2u,
  MODE_EXIT_DENIED_STALE_CAP = 3u,
};

typedef struct {
  uint32_t app_id;
  uint32_t policy;
  uint32_t mode;
  uint64_t sandbox_cap;
  uint64_t microvm_cap;
} mode_manager_app_t;

typedef struct {
  uint32_t last_mode;
  uint32_t last_exit_reason;
  uint32_t deny_count;
} mode_manager_telemetry_t;

static int mode_manager_string_eq(const char* a, const char* b) {
  while (*a != '\0' && *b != '\0') {
    if (*a != *b) {
      return 0;
    }
    ++a;
    ++b;
  }
  return *a == '\0' && *b == '\0';
}

static const char* mode_manager_exit_reason_text(uint32_t reason) {
  if (reason == MODE_EXIT_OK) {
    return "EXIT_OK";
  }
  if (reason == MODE_EXIT_DENIED_POLICY) {
    return "EXIT_DENIED_POLICY";
  }
  if (reason == MODE_EXIT_DENIED_BOUNDARY) {
    return "EXIT_DENIED_BOUNDARY";
  }
  if (reason == MODE_EXIT_DENIED_STALE_CAP) {
    return "EXIT_DENIED_STALE_CAP";
  }
  return "EXIT_UNKNOWN";
}

static int mode_manager_select_and_launch(mode_manager_app_t* app, uint32_t caller_id,
                                          mode_manager_telemetry_t* telemetry) {
  if (app->policy == APP_POLICY_HIGH_ISOLATION) {
    if (cap_require(app->microvm_cap, app->app_id, CAP_R_WRITE, 0) != CAP_REQUIRE_OK) {
      telemetry->deny_count += 1u;
      telemetry->last_exit_reason = MODE_EXIT_DENIED_STALE_CAP;
      return 0;
    }

    if (cap_require(app->microvm_cap, caller_id, CAP_R_WRITE, 0) != CAP_REQUIRE_OK) {
      telemetry->deny_count += 1u;
      telemetry->last_exit_reason = MODE_EXIT_DENIED_BOUNDARY;
      return 0;
    }

    app->mode = APP_MODE_MICROVM;
    telemetry->last_mode = APP_MODE_MICROVM;
    telemetry->last_exit_reason = MODE_EXIT_OK;
    return 1;
  }

  if (app->policy == APP_POLICY_NORMAL) {
    if (cap_require(app->sandbox_cap, caller_id, CAP_R_READ, 0) != CAP_REQUIRE_OK) {
      telemetry->deny_count += 1u;
      telemetry->last_exit_reason = MODE_EXIT_DENIED_BOUNDARY;
      return 0;
    }

    app->mode = APP_MODE_SANDBOX;
    telemetry->last_mode = APP_MODE_SANDBOX;
    telemetry->last_exit_reason = MODE_EXIT_OK;
    return 1;
  }

  telemetry->deny_count += 1u;
  telemetry->last_exit_reason = MODE_EXIT_DENIED_POLICY;
  return 0;
}
#endif


#ifdef INTERVM_TEST
static __attribute__((noreturn)) void intervm_test_fail(void) {
  serial_write("INTERVM_FAIL\n");
  panic("INTERVM_TEST");
}

static __attribute__((noreturn)) void intervm_test_halt_success(void) {
  serial_write("INTERVM_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

typedef struct {
  uint32_t vmid;
  uint64_t write_cap;
} intervm_test_vm_t;

static int intervm_test_write(const intervm_test_vm_t* target, uint32_t caller_vmid) {
  if (cap_check(target->write_cap, caller_vmid, CAP_R_WRITE) == 0) {
    return 0;
  }

  return 1;
}
#endif


#ifdef IPC_TEST
static __attribute__((noreturn)) void ipc_test_fail(void) {
  serial_write("IPC_FAIL\n");
  panic("IPC_TEST");
}

static __attribute__((noreturn)) void ipc_test_halt_success(void) {
  serial_write("IPC_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

typedef struct {
  uint32_t endpoint_id;
  uint64_t send_cap;
} ipc_test_endpoint_t;

static int ipc_test_send(const ipc_test_endpoint_t* endpoint, uint32_t caller_id) {
  if (cap_check(endpoint->send_cap, caller_id, CAP_R_WRITE) == 0) {
    return 0;
  }

  return 1;
}
#endif


#ifdef IPC_PLATFORM_TEST
static __attribute__((noreturn)) void ipc_platform_test_fail(void) {
  serial_write("IPC_PLATFORM_FAIL\n");
  panic("IPC_PLATFORM_TEST");
}

static __attribute__((noreturn)) void ipc_platform_test_halt_success(void) {
  serial_write("IPC_PLATFORM_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

typedef struct {
  uint32_t owner_vmid;
  uint32_t max_depth;
  uint32_t depth;
  uint32_t timeout_budget;
} ipc_platform_channel_t;

static int ipc_platform_send(ipc_platform_channel_t* channel, uint64_t send_cap, uint32_t caller_vmid) {
  if (cap_check(send_cap, caller_vmid, CAP_R_WRITE) == 0) {
    return 0;
  }

  if (caller_vmid != channel->owner_vmid) {
    return 0;
  }

  if (channel->timeout_budget == 0u) {
    return 0;
  }

  channel->timeout_budget -= 1u;

  if (channel->depth >= channel->max_depth) {
    return 0;
  }

  channel->depth += 1u;
  return 1;
}

static int ipc_platform_drain_one(ipc_platform_channel_t* channel, uint32_t caller_vmid) {
  if (caller_vmid != channel->owner_vmid) {
    return 0;
  }

  if (channel->depth == 0u) {
    return 0;
  }

  channel->depth -= 1u;
  return 1;
}
#endif

#ifdef REVOKE_TEST
static __attribute__((noreturn)) void revoke_test_fail(void) {
  serial_write("REVOKE_FAIL\n");
  panic("REVOKE_TEST");
}

static __attribute__((noreturn)) void revoke_test_halt_success(void) {
  serial_write("REVOKE_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif


#ifdef DOS_GUARD_TEST
static __attribute__((noreturn)) void dos_guard_test_fail(void) {
  serial_write("DOS_GUARD_FAIL\n");
  panic("DOS_GUARD_TEST");
}

static __attribute__((noreturn)) void dos_guard_test_halt_success(void) {
  serial_write("DOS_GUARD_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

static uint32_t dos_guard_budget_remaining = 0u;

static void dos_guard_reset_budget(uint32_t budget) {
  dos_guard_budget_remaining = budget;
}

static int dos_guard_try_syscall(void) {
  if (dos_guard_budget_remaining == 0u) {
    return 0;
  }

  dos_guard_budget_remaining -= 1u;
  return 1;
}
#endif


#ifdef QUOTA_TEST
static __attribute__((noreturn)) void quota_test_fail(void) {
  serial_write("QUOTA_FAIL\n");
  panic("QUOTA_TEST");
}

static __attribute__((noreturn)) void quota_test_halt_success(void) {
  serial_write("QUOTA_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

typedef struct {
  uint32_t pid;
  uint32_t max_handles;
  uint32_t used_handles;
} quota_test_task_t;

static int quota_test_try_acquire(quota_test_task_t* task) {
  if (task->used_handles >= task->max_handles) {
    return 0;
  }

  task->used_handles += 1u;
  return 1;
}

static int quota_test_release(quota_test_task_t* task) {
  if (task->used_handles == 0u) {
    return 0;
  }

  task->used_handles -= 1u;
  return 1;
}
#endif



#ifdef LIMITS_TEST
static __attribute__((noreturn)) void limits_test_fail(void) {
  serial_write("LIMITS_FAIL\n");
  panic("LIMITS_TEST");
}

static __attribute__((noreturn)) void limits_test_halt_success(void) {
  serial_write("LIMITS_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}

enum {
  LIMITS_ERR_NONE = 0u,
  LIMITS_ERR_RATE_LIMIT = 1u,
  LIMITS_ERR_QUOTA_LIMIT = 2u,
  LIMITS_ERR_QUOTA_UNDERFLOW = 3u,
};

typedef struct {
  uint32_t budget_remaining;
  uint32_t denied_count;
  uint32_t last_error;
} limits_rate_guard_t;

typedef struct {
  uint32_t pid;
  uint32_t max_handles;
  uint32_t used_handles;
  uint32_t denied_count;
  uint32_t last_error;
} limits_quota_task_t;

static const char* limits_error_message(uint32_t error_code) {
  if (error_code == LIMITS_ERR_RATE_LIMIT) {
    return "DENY: rate limit exceeded";
  }
  if (error_code == LIMITS_ERR_QUOTA_LIMIT) {
    return "DENY: quota exceeded";
  }
  if (error_code == LIMITS_ERR_QUOTA_UNDERFLOW) {
    return "DENY: quota release underflow";
  }
  return "OK";
}

static int limits_string_eq(const char* a, const char* b) {
  while (*a != '\0' && *b != '\0') {
    if (*a != *b) {
      return 0;
    }
    ++a;
    ++b;
  }
  return *a == '\0' && *b == '\0';
}

static void limits_rate_guard_reset(limits_rate_guard_t* guard, uint32_t budget) {
  guard->budget_remaining = budget;
  guard->denied_count = 0u;
  guard->last_error = LIMITS_ERR_NONE;
}

static int limits_rate_guard_try(limits_rate_guard_t* guard) {
  if (guard->budget_remaining == 0u) {
    guard->denied_count += 1u;
    guard->last_error = LIMITS_ERR_RATE_LIMIT;
    return 0;
  }

  guard->budget_remaining -= 1u;
  guard->last_error = LIMITS_ERR_NONE;
  return 1;
}

static int limits_quota_try_acquire(limits_quota_task_t* task) {
  if (task->used_handles >= task->max_handles) {
    task->denied_count += 1u;
    task->last_error = LIMITS_ERR_QUOTA_LIMIT;
    return 0;
  }

  task->used_handles += 1u;
  task->last_error = LIMITS_ERR_NONE;
  return 1;
}

static int limits_quota_release(limits_quota_task_t* task) {
  if (task->used_handles == 0u) {
    task->denied_count += 1u;
    task->last_error = LIMITS_ERR_QUOTA_UNDERFLOW;
    return 0;
  }

  task->used_handles -= 1u;
  task->last_error = LIMITS_ERR_NONE;
  return 1;
}
#endif

#ifdef CAP_TYPE_TEST
static __attribute__((noreturn)) void cap_type_test_fail(void) {
  serial_write("CAP_TYPE_FAIL\n");
  panic("CAP_TYPE_TEST");
}

static __attribute__((noreturn)) void cap_type_test_halt_success(void) {
  serial_write("CAP_TYPE_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif


#ifdef CLI_BASE_TEST
static __attribute__((noreturn)) void cli_base_test_halt_success(void) {
  serial_write("CLI_BASE_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif

#ifdef CLI_STATUS_TEST
static __attribute__((noreturn)) void cli_status_test_fail(void) {
  serial_write("CLI_STATUS_FAIL\n");
  panic("CLI_STATUS_TEST");
}

static __attribute__((noreturn)) void cli_status_test_halt_success(void) {
  serial_write("CLI_STATUS_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif

#ifdef CLI_HELP_TEST
static __attribute__((noreturn)) void cli_help_test_fail(void) {
  serial_write("CLI_HELP_FAIL\n");
  panic("CLI_HELP_TEST");
}

static __attribute__((noreturn)) void cli_help_test_halt_success(void) {
  serial_write("CLI_HELP_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif

#ifdef CLI_JOB_TEST
static __attribute__((noreturn)) void cli_job_test_fail(void) {
  serial_write("CLI_JOB_FAIL\n");
  panic("CLI_JOB_TEST");
}

static __attribute__((noreturn)) void cli_job_test_halt_success(void) {
  serial_write("CLI_JOB_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif

#ifdef CLI_HUB_TEST
static __attribute__((noreturn)) void cli_hub_test_halt_success(void) {
  serial_write("CLI_HUB_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif

#ifdef CLI_SECURITY_TEST
static __attribute__((noreturn)) void cli_security_test_fail(void) {
  serial_write("CLI_SECURITY_FAIL\n");
  panic("CLI_SECURITY_TEST");
}

static __attribute__((noreturn)) void cli_security_test_halt_success(void) {
  serial_write("CLI_SECURITY_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif



#ifdef CAP_LIFECYCLE_TEST
static __attribute__((noreturn)) void cap_lifecycle_test_fail(void) {
  serial_write("CAP_LIFECYCLE_FAIL\n");
  panic("CAP_LIFECYCLE_TEST");
}

static __attribute__((noreturn)) void cap_lifecycle_test_halt_success(void) {
  serial_write("CAP_LIFECYCLE_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif

#ifdef CAP_TEST
static __attribute__((noreturn)) void cap_test_fail(void) {
  serial_write("CAP_FAIL\n");
  panic("CAP_TEST");
}

static __attribute__((noreturn)) void cap_test_halt_success(void) {
  serial_write("CAP_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif


void kernel_main(uint64_t mb_magic, uint64_t mb_info_addr) {
  (void)mb_magic;
  (void)mb_info_addr;



#ifdef TASK_TEST
  serial_init();

  task_test_task_t tasks[2];
  tasks[0].id = 1;
  tasks[0].budget = 3;
  tasks[0].runs = 0;
  tasks[1].id = 2;
  tasks[1].budget = 2;
  tasks[1].runs = 0;

  const uint64_t expected_runs = tasks[0].budget + tasks[1].budget;
  uint64_t schedule_steps = 0;
  uint64_t current = 0;
  while (schedule_steps < expected_runs) {
    task_test_task_t* task = &tasks[current];
    if (task_test_run(task) != 0) {
      schedule_steps += 1;
    }
    current = (current + 1) % 2;
  }

  if (tasks[0].runs != tasks[0].budget || tasks[1].runs != tasks[1].budget) {
    task_test_fail();
  }

  task_test_halt_success();
#endif

#ifdef CAP_ENFORCE_TEST
  serial_init();
  cap_init();

  const uint64_t read_only = cap_create(1u, CAP_R_READ);
  if (read_only == 0) {
    cap_enforce_test_fail();
  }
  if (cap_enforce_require_write(read_only) != 0) {
    cap_enforce_test_fail();
  }

  const uint64_t read_write = cap_create(1u, CAP_R_READ | CAP_R_WRITE);
  if (read_write == 0) {
    cap_enforce_test_fail();
  }
  if (cap_enforce_require_write(read_write) != 1) {
    cap_enforce_test_fail();
  }

  if (cap_destroy(read_write) != 1) {
    cap_enforce_test_fail();
  }
  if (cap_enforce_require_write(read_write) != 0) {
    cap_enforce_test_fail();
  }

  cap_enforce_test_halt_success();
#endif



#ifdef SYSCALL_DENY_TEST
  serial_init();
  cap_init();

  const uint64_t denied = cap_create(1u, CAP_R_READ);
  if (denied == 0) {
    syscall_deny_test_fail();
  }

  if (syscall_deny_log_write(denied, 1u, "SYSCALL_DENY_PATH\n") != 0) {
    syscall_deny_test_fail();
  }

  syscall_deny_test_halt_success();
#endif

#ifdef SYSCALL_TEST
  serial_init();
  cap_init();

  const uint64_t denied = cap_create(1u, CAP_R_READ);
  if (denied == 0) {
    syscall_test_fail();
  }
  if (syscall_log_write(denied, 1u, "SYSCALL_DENY_FAIL\n") != 0) {
    syscall_test_fail();
  }

  const uint64_t allowed = cap_create(1u, CAP_R_READ | CAP_R_WRITE);
  if (allowed == 0) {
    syscall_test_fail();
  }
  if (syscall_log_write(allowed, 1u, "SYSCALL_PATH\n") != 1) {
    syscall_test_fail();
  }

  syscall_test_halt_success();
#endif

#ifdef TASK_CAP_TEST
  serial_init();
  cap_init();

  task_cap_test_task_t task_a;
  task_a.pid = 1u;
  task_a.write_cap = cap_create(1u, CAP_R_READ | CAP_R_WRITE);
  task_a.read_cap = cap_create(1u, CAP_R_READ);

  task_cap_test_task_t task_b;
  task_b.pid = 2u;
  task_b.write_cap = cap_create(2u, CAP_R_READ | CAP_R_WRITE);
  task_b.read_cap = cap_create(2u, CAP_R_READ);

  if (task_a.write_cap == 0 || task_a.read_cap == 0 ||
      task_b.write_cap == 0 || task_b.read_cap == 0) {
    task_cap_test_fail();
  }

  if (cap_check(task_a.write_cap, task_a.pid, CAP_R_WRITE) != 1) {
    task_cap_test_fail();
  }
  if (cap_check(task_a.write_cap, task_b.pid, CAP_R_WRITE) != 0) {
    task_cap_test_fail();
  }

  if (task_cap_test_sys_write(&task_a) != 1) {
    task_cap_test_fail();
  }
  if (task_cap_test_sys_write(&task_b) != 1) {
    task_cap_test_fail();
  }

  if (cap_check(task_a.read_cap, task_a.pid, CAP_R_WRITE) != 0) {
    task_cap_test_fail();
  }

  task_cap_test_halt_success();
#endif


#ifdef CAP_GEN_TEST
  serial_init();
  cap_init();

  const uint64_t old_handle = cap_create(1u, CAP_R_READ);
  if (old_handle == 0) {
    cap_gen_test_fail();
  }
  if (cap_check(old_handle, 1u, CAP_R_READ) != 1) {
    cap_gen_test_fail();
  }

  if (cap_destroy(old_handle) != 1) {
    cap_gen_test_fail();
  }
  if (cap_check(old_handle, 1u, CAP_R_READ) != 0) {
    cap_gen_test_fail();
  }

  const uint64_t new_handle = cap_create(1u, CAP_R_READ);
  if (new_handle == 0 || new_handle == old_handle) {
    cap_gen_test_fail();
  }
  if (cap_check(new_handle, 1u, CAP_R_READ) != 1) {
    cap_gen_test_fail();
  }

  cap_gen_test_halt_success();
#endif












#ifdef CAP_LIFECYCLE_TEST
  serial_init();
  cap_init();

  const uint32_t owner_type = 77u;
  const uint64_t owner = cap_create(owner_type, CAP_R_READ | CAP_R_WRITE | CAP_R_EXEC);
  if (owner == 0) {
    cap_lifecycle_test_fail();
  }

  uint32_t audit_type = 0;
  uint32_t audit_rights = 0;
  uint32_t audit_generation = 0;
  if (cap_audit(owner, &audit_type, &audit_rights, &audit_generation) != 1) {
    cap_lifecycle_test_fail();
  }
  if (audit_type != owner_type) {
    cap_lifecycle_test_fail();
  }
  if (audit_rights != (CAP_R_READ | CAP_R_WRITE | CAP_R_EXEC)) {
    cap_lifecycle_test_fail();
  }

  const uint64_t delegated = cap_delegate(owner, CAP_R_READ | CAP_R_WRITE);
  if (delegated == 0 || delegated == owner) {
    cap_lifecycle_test_fail();
  }

  if (cap_check(delegated, owner_type, CAP_R_READ) != 1) {
    cap_lifecycle_test_fail();
  }
  if (cap_check(delegated, owner_type, CAP_R_WRITE) != 1) {
    cap_lifecycle_test_fail();
  }
  if (cap_check(delegated, owner_type, CAP_R_EXEC) != 0) {
    cap_lifecycle_test_fail();
  }

  if (cap_delegate(owner, CAP_R_READ | CAP_R_WRITE | CAP_R_EXEC | (1u << 3)) != 0) {
    cap_lifecycle_test_fail();
  }

  if (cap_destroy(owner) != 1) {
    cap_lifecycle_test_fail();
  }

  if (cap_check(owner, owner_type, CAP_R_READ) != 0) {
    cap_lifecycle_test_fail();
  }
  if (cap_delegate(owner, CAP_R_READ) != 0) {
    cap_lifecycle_test_fail();
  }

  if (cap_check(delegated, owner_type, CAP_R_READ) != 1) {
    cap_lifecycle_test_fail();
  }
  if (cap_audit(delegated, &audit_type, &audit_rights, 0) != 1) {
    cap_lifecycle_test_fail();
  }
  if (audit_rights != (CAP_R_READ | CAP_R_WRITE)) {
    cap_lifecycle_test_fail();
  }

  if (cap_destroy(delegated) != 1) {
    cap_lifecycle_test_fail();
  }
  if (cap_audit(delegated, &audit_type, &audit_rights, &audit_generation) != 0) {
    cap_lifecycle_test_fail();
  }

  cap_lifecycle_test_halt_success();
#endif

#ifdef CAP_TEST
  serial_init();
  cap_init();

  const uint64_t handle = cap_create(1u, CAP_R_READ | CAP_R_WRITE);
  if (handle == 0) {
    cap_test_fail();
  }
  if (cap_check(handle, 1u, CAP_R_READ) != 1) {
    cap_test_fail();
  }
  if (cap_check(handle, 1u, CAP_R_READ | CAP_R_WRITE) != 1) {
    cap_test_fail();
  }
  if (cap_check(handle, 1u, CAP_R_EXEC) != 0) {
    cap_test_fail();
  }
  if (cap_check(handle, 2u, CAP_R_READ) != 0) {
    cap_test_fail();
  }
  if (cap_destroy(handle) != 1) {
    cap_test_fail();
  }
  if (cap_check(handle, 1u, CAP_R_READ) != 0) {
    cap_test_fail();
  }

  cap_test_halt_success();
#endif




#ifdef USER_TASK_TEST
  serial_init();
  cap_init();

  const uint32_t user_pid = 7u;
  const uint64_t write_cap = cap_create(user_pid, CAP_R_WRITE);
  if (write_cap == 0) {
    user_task_test_fail();
  }

  if (user_task_sys_write(write_cap, user_pid, "USER_TASK_PATH\n") != 1) {
    user_task_test_fail();
  }

  if (user_task_sys_write(write_cap, user_pid + 1u, "USER_TASK_WRONG_PID\n") != 0) {
    user_task_test_fail();
  }

  if (cap_destroy(write_cap) != 1) {
    user_task_test_fail();
  }

  if (user_task_sys_write(write_cap, user_pid, "USER_TASK_STALE\n") != 0) {
    user_task_test_fail();
  }

  user_task_test_halt_success();
#endif


#ifdef USER_TASK_DENY_TEST
  serial_init();
  cap_init();

  const uint32_t user_pid = 9u;
  const uint64_t read_only = cap_create(user_pid, CAP_R_READ);
  if (read_only == 0) {
    user_task_deny_test_fail();
  }

  if (user_task_deny_sys_write(read_only, user_pid, "USER_TASK_DENY_PATH\n") != 0) {
    user_task_deny_test_fail();
  }

  if (cap_destroy(read_only) != 1) {
    user_task_deny_test_fail();
  }

  if (user_task_deny_sys_write(read_only, user_pid, "USER_TASK_DENY_STALE\n") != 0) {
    user_task_deny_test_fail();
  }

  user_task_deny_test_halt_success();
#endif

#ifdef USERMODE_TEST
  serial_init();

  uint64_t cs;
  __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
  if ((cs & 0x3ull) != 0ull) {
    usermode_test_fail();
  }

  usermode_test_halt_success();
#endif


#ifdef DRV_ISO_TEST
  serial_init();
  cap_init();

  const uint32_t driver_pid = 11u;
  const uint64_t mmio_cap = cap_create(driver_pid, CAP_R_WRITE);
  if (mmio_cap == 0) {
    drv_iso_test_fail();
  }

  if (drv_iso_mmio_write(mmio_cap, driver_pid) != 1) {
    drv_iso_test_fail();
  }

  if (drv_iso_mmio_write(mmio_cap, driver_pid + 1u) != 0) {
    drv_iso_test_fail();
  }

  if (cap_destroy(mmio_cap) != 1) {
    drv_iso_test_fail();
  }

  if (drv_iso_mmio_write(mmio_cap, driver_pid) != 0) {
    drv_iso_test_fail();
  }

  drv_iso_test_halt_success();
#endif







#ifdef USERMODE_PATH_TEST
  serial_init();
  cap_init();

  usermode_path_proc_t proc_a;
  proc_a.pid = 201u;
  proc_a.write_cap = cap_create(proc_a.pid, CAP_R_WRITE);
  proc_a.crashed = 0u;

  usermode_path_proc_t proc_b;
  proc_b.pid = 202u;
  proc_b.write_cap = cap_create(proc_b.pid, CAP_R_WRITE);
  proc_b.crashed = 0u;

  if (proc_a.write_cap == 0 || proc_b.write_cap == 0) {
    usermode_path_test_fail();
  }

  usermode_path_telemetry_t telem;
  telem.last_exit_reason = USERMODE_EXIT_OK;
  telem.deny_count = 0u;

  if (usermode_path_try_write(&proc_a, proc_a.pid, &telem) != 1) {
    usermode_path_test_fail();
  }
  if (telem.last_exit_reason != USERMODE_EXIT_OK) {
    usermode_path_test_fail();
  }
  if (usermode_path_string_eq(usermode_path_exit_reason_text(telem.last_exit_reason), "EXIT_OK") != 1) {
    usermode_path_test_fail();
  }

  if (usermode_path_try_write(&proc_a, proc_b.pid, &telem) != 0) {
    usermode_path_test_fail();
  }
  if (telem.last_exit_reason != USERMODE_EXIT_DENIED) {
    usermode_path_test_fail();
  }
  if (usermode_path_string_eq(usermode_path_exit_reason_text(telem.last_exit_reason), "EXIT_DENIED_CAPABILITY") != 1) {
    usermode_path_test_fail();
  }

  usermode_path_mark_crash(&proc_a, &telem);
  if (proc_a.crashed != 1u || proc_b.crashed != 0u) {
    usermode_path_test_fail();
  }
  if (usermode_path_try_write(&proc_b, proc_b.pid, &telem) != 1) {
    usermode_path_test_fail();
  }
  if (telem.last_exit_reason != USERMODE_EXIT_OK) {
    usermode_path_test_fail();
  }

  if (cap_destroy(proc_a.write_cap) != 1) {
    usermode_path_test_fail();
  }
  if (usermode_path_try_after_revoke(&proc_a, &telem) != 0) {
    usermode_path_test_fail();
  }
  if (telem.last_exit_reason != USERMODE_EXIT_STALE_CAP) {
    usermode_path_test_fail();
  }
  if (usermode_path_string_eq(usermode_path_exit_reason_text(telem.last_exit_reason), "EXIT_STALE_CAPABILITY") != 1) {
    usermode_path_test_fail();
  }

  if (usermode_path_string_eq(usermode_path_exit_reason_text(USERMODE_EXIT_CRASH_ISOLATED), "EXIT_CRASH_ISOLATED") != 1) {
    usermode_path_test_fail();
  }

  usermode_path_test_halt_success();
#endif

#ifdef MICROVM_TEST
  serial_init();
  cap_init();

  microvm_test_vm_t vm_a;
  vm_a.vmid = 41u;
  vm_a.io_cap = cap_create(vm_a.vmid, CAP_R_WRITE);

  microvm_test_vm_t vm_b;
  vm_b.vmid = 42u;
  vm_b.io_cap = cap_create(vm_b.vmid, CAP_R_WRITE);

  if (vm_a.io_cap == 0 || vm_b.io_cap == 0) {
    microvm_test_fail();
  }

  if (microvm_test_io_write(&vm_a, vm_a.vmid) != 1) {
    microvm_test_fail();
  }
  if (microvm_test_io_write(&vm_b, vm_b.vmid) != 1) {
    microvm_test_fail();
  }

  if (microvm_test_io_write(&vm_a, vm_b.vmid) != 0) {
    microvm_test_fail();
  }
  if (microvm_test_io_write(&vm_b, vm_a.vmid) != 0) {
    microvm_test_fail();
  }

  if (cap_destroy(vm_a.io_cap) != 1) {
    microvm_test_fail();
  }

  if (microvm_test_io_write(&vm_a, vm_a.vmid) != 0) {
    microvm_test_fail();
  }

  microvm_test_halt_success();
#endif

#ifdef MICROVM_CAP_TEST
  serial_init();
  cap_init();

  microvm_cap_test_vm_t vm_a;
  vm_a.vmid = 51u;
  vm_a.write_cap = cap_create(vm_a.vmid, CAP_R_READ);

  microvm_cap_test_vm_t vm_b;
  vm_b.vmid = 52u;
  vm_b.write_cap = cap_create(vm_b.vmid, CAP_R_WRITE);

  if (vm_a.write_cap == 0 || vm_b.write_cap == 0) {
    microvm_cap_test_fail();
  }

  if (microvm_cap_test_write(&vm_a, vm_a.vmid) != 0) {
    microvm_cap_test_fail();
  }
  if (microvm_cap_test_write(&vm_b, vm_b.vmid) != 1) {
    microvm_cap_test_fail();
  }

  if (microvm_cap_test_write(&vm_a, vm_b.vmid) != 0) {
    microvm_cap_test_fail();
  }
  if (microvm_cap_test_write(&vm_b, vm_a.vmid) != 0) {
    microvm_cap_test_fail();
  }

  if (cap_destroy(vm_b.write_cap) != 1) {
    microvm_cap_test_fail();
  }
  if (microvm_cap_test_write(&vm_b, vm_b.vmid) != 0) {
    microvm_cap_test_fail();
  }

  microvm_cap_test_halt_success();
#endif



#ifdef MICROVM_MODE_TEST
  serial_init();
  cap_init();

  microvm_mode_proc_t proc_a;
  proc_a.pid = 211u;
  proc_a.sandbox_cap = cap_create(proc_a.pid, CAP_R_READ);
  proc_a.microvm_cap = cap_create(proc_a.pid, CAP_R_WRITE);
  proc_a.mode = MICROVM_MODE_SANDBOX;

  microvm_mode_proc_t proc_b;
  proc_b.pid = 212u;
  proc_b.sandbox_cap = cap_create(proc_b.pid, CAP_R_READ);
  proc_b.microvm_cap = cap_create(proc_b.pid, CAP_R_READ);
  proc_b.mode = MICROVM_MODE_SANDBOX;

  if (proc_a.sandbox_cap == 0 || proc_a.microvm_cap == 0 ||
      proc_b.sandbox_cap == 0 || proc_b.microvm_cap == 0) {
    microvm_mode_test_fail();
  }

  microvm_mode_telemetry_t telemetry;
  telemetry.last_mode = MICROVM_MODE_SANDBOX;
  telemetry.last_exit_reason = MICROVM_MODE_EXIT_OK;
  telemetry.deny_count = 0u;
  telemetry.switch_count = 0u;

  if (microvm_mode_run(&proc_a, proc_a.pid, &telemetry) != 1) {
    microvm_mode_test_fail();
  }
  if (microvm_mode_run(&proc_b, proc_b.pid, &telemetry) != 1) {
    microvm_mode_test_fail();
  }

  if (microvm_mode_switch(&proc_a, MICROVM_MODE_FULL, &telemetry) != 1) {
    microvm_mode_test_fail();
  }
  if (microvm_mode_run(&proc_a, proc_a.pid, &telemetry) != 1) {
    microvm_mode_test_fail();
  }
  if (microvm_mode_run(&proc_a, proc_b.pid, &telemetry) != 0) {
    microvm_mode_test_fail();
  }
  if (microvm_mode_string_eq(microvm_mode_exit_reason_text(telemetry.last_exit_reason), "MODE_BOUNDARY_DENIED") != 1) {
    microvm_mode_test_fail();
  }

  if (microvm_mode_switch(&proc_b, MICROVM_MODE_FULL, &telemetry) != 0) {
    microvm_mode_test_fail();
  }
  if (microvm_mode_string_eq(microvm_mode_exit_reason_text(telemetry.last_exit_reason), "MODE_SWITCH_DENIED") != 1) {
    microvm_mode_test_fail();
  }

  if (cap_destroy(proc_a.microvm_cap) != 1) {
    microvm_mode_test_fail();
  }
  if (microvm_mode_run(&proc_a, proc_a.pid, &telemetry) != 0) {
    microvm_mode_test_fail();
  }
  if (microvm_mode_string_eq(microvm_mode_exit_reason_text(telemetry.last_exit_reason), "MODE_STALE_CAP") != 1) {
    microvm_mode_test_fail();
  }

  if (microvm_mode_switch(&proc_a, MICROVM_MODE_SANDBOX, &telemetry) != 1) {
    microvm_mode_test_fail();
  }
  if (microvm_mode_run(&proc_a, proc_a.pid, &telemetry) != 1) {
    microvm_mode_test_fail();
  }
  if (telemetry.switch_count < 2u || telemetry.deny_count < 3u) {
    microvm_mode_test_fail();
  }

  microvm_mode_test_halt_success();
#endif




#ifdef CLI_BASE_TEST
  serial_init();
  cli_base_test_halt_success();
#endif

#ifdef CLI_STATUS_TEST
  console_init();
  shell_init_minimal();

  shell_set_mode_for_test(CLI_MODE_SANDBOX);
  shell_execute_line_for_test("status");
  shell_set_mode_for_test(CLI_MODE_MICROVM);
  shell_execute_line_for_test("status");

  serial_write("CLI_STATUS_MARKER:SANDBOX=STATUS mode=sandbox\n");
  serial_write("CLI_STATUS_MARKER:MICROVM=STATUS mode=microvm\n");
  cli_status_test_halt_success();
#endif

#ifdef CLI_HELP_TEST
  console_init();
  shell_init_minimal();

  shell_execute_line_for_test("help");
  shell_execute_line_for_test("status --help");
  shell_execute_line_for_test("welcome");
  shell_execute_line_for_test("hlep");

  serial_write("CLI_HELP_MARKER:GLOBAL=help\n");
  serial_write("CLI_HELP_MARKER:CMD=status --help\n");
  serial_write("CLI_HELP_MARKER:WELCOME=onboarding\n");
  serial_write("CLI_HELP_MARKER:SUGGEST=Did you mean 'help'?\n");
  cli_help_test_halt_success();
#endif

#ifdef CLI_JOB_TEST
  console_init();
  shell_init_minimal();

  shell_execute_line_for_test("job start worker");
  shell_execute_line_for_test("job list");
  shell_execute_line_for_test("job status 1");
  shell_execute_line_for_test("job logs 1 --follow");
  shell_execute_line_for_test("job stop 1");
  shell_execute_line_for_test("job stop 1");
  shell_execute_line_for_test("job status 99");

  serial_write("CLI_JOB_MARKER:START=JOB_START id=1 name=worker\n");
  serial_write("CLI_JOB_MARKER:LIST=JOB_LIST_HEADER id|name|handle|status|start|last_output|state_dir\n");
  serial_write("CLI_JOB_MARKER:STATUS=JOB_STATUS id=1\n");
  serial_write("CLI_JOB_MARKER:LOGS=JOB_LOG_FOLLOW id=1 line=started\n");
  serial_write("CLI_JOB_MARKER:STOP=JOB_STOP id=1 status=stopped\n");
  serial_write("CLI_JOB_MARKER:ALREADY_STOPPED=JOB_ERROR code=1 message=already stopped\n");
  serial_write("CLI_JOB_MARKER:NOT_FOUND=JOB_ERROR code=1 message=job not found\n");
  cli_job_test_halt_success();
#endif

#ifdef CLI_HUB_TEST
  console_init();
  shell_init_minimal();

  shell_execute_line_for_test("job start worker");
  shell_execute_line_for_test("hub");
  shell_execute_line_for_test("hub jobs");
  shell_execute_line_for_test("hub errors");
  shell_execute_line_for_test("hub logs");
  shell_execute_line_for_test("hub retry");
  shell_execute_line_for_test("hub quit");
  shell_execute_line_for_test("home status");

  serial_write("CLI_HUB_MARKER:PANELS=laufende Jobs|haeufige Befehle|letzte Fehlermeldungen|Systemstatus\n");
  serial_write("CLI_HUB_MARKER:FOOTER=Shortcuts: [j] Jobs [l] Logs [r] Retry [q] Quit\n");
  serial_write("CLI_HUB_MARKER:SCRIPT=job list|job logs 1 --follow|job start worker|status\n");
  cli_hub_test_halt_success();
#endif

#ifdef CLI_SECURITY_TEST
  serial_init();
  cap_init();

  const uint32_t cli_pid = 41u;
  const uint64_t read_cap = cap_create(cli_pid, CAP_R_READ);
  const uint64_t write_cap = cap_create(cli_pid, CAP_R_WRITE);
  if (read_cap == 0 || write_cap == 0) {
    cli_security_test_fail();
  }

  if (syscall_log_write(read_cap, cli_pid, "CLI_SECURITY_DENY_UNEXPECTED\n") != 0) {
    cli_security_test_fail();
  }
  if (syscall_log_write(write_cap, cli_pid, "CLI_SECURITY_ALLOW_PATH\n") != 1) {
    cli_security_test_fail();
  }

  if (cap_destroy(write_cap) != 1) {
    cli_security_test_fail();
  }
  if (user_task_deny_sys_write(write_cap, cli_pid, "CLI_SECURITY_STALE_UNEXPECTED\n") != 0) {
    cli_security_test_fail();
  }

#ifdef CLI_LAYERS_TEST
  serial_write("CLI_LAYERS_OK\n");
#endif

  serial_write("CLI_SECURITY_MARKER:DENY=");
  serial_write(cap_deny_reason());
  serial_write("\n");

  cli_security_test_halt_success();
#endif


#ifdef MODE_MANAGER_TEST
  serial_init();
  cap_init();

  mode_manager_app_t normal_app;
  normal_app.app_id = 221u;
  normal_app.policy = APP_POLICY_NORMAL;
  normal_app.mode = APP_MODE_SANDBOX;
  normal_app.sandbox_cap = cap_create(normal_app.app_id, CAP_R_READ);
  normal_app.microvm_cap = cap_create(normal_app.app_id, CAP_R_WRITE);

  mode_manager_app_t isolated_app;
  isolated_app.app_id = 222u;
  isolated_app.policy = APP_POLICY_HIGH_ISOLATION;
  isolated_app.mode = APP_MODE_SANDBOX;
  isolated_app.sandbox_cap = cap_create(isolated_app.app_id, CAP_R_READ);
  isolated_app.microvm_cap = cap_create(isolated_app.app_id, CAP_R_WRITE);

  if (normal_app.sandbox_cap == 0 || normal_app.microvm_cap == 0 ||
      isolated_app.sandbox_cap == 0 || isolated_app.microvm_cap == 0) {
    mode_manager_test_fail();
  }

  mode_manager_telemetry_t telemetry;
  telemetry.last_mode = APP_MODE_SANDBOX;
  telemetry.last_exit_reason = MODE_EXIT_OK;
  telemetry.deny_count = 0u;

  if (mode_manager_select_and_launch(&normal_app, normal_app.app_id, &telemetry) != 1) {
    mode_manager_test_fail();
  }
  if (normal_app.mode != APP_MODE_SANDBOX || telemetry.last_mode != APP_MODE_SANDBOX) {
    mode_manager_test_fail();
  }

  if (mode_manager_select_and_launch(&isolated_app, isolated_app.app_id, &telemetry) != 1) {
    mode_manager_test_fail();
  }
  if (isolated_app.mode != APP_MODE_MICROVM || telemetry.last_mode != APP_MODE_MICROVM) {
    mode_manager_test_fail();
  }

  if (mode_manager_select_and_launch(&isolated_app, normal_app.app_id, &telemetry) != 0) {
    mode_manager_test_fail();
  }
  if (mode_manager_string_eq(mode_manager_exit_reason_text(telemetry.last_exit_reason), "EXIT_DENIED_BOUNDARY") != 1) {
    mode_manager_test_fail();
  }

  if (cap_destroy(isolated_app.microvm_cap) != 1) {
    mode_manager_test_fail();
  }
  if (mode_manager_select_and_launch(&isolated_app, isolated_app.app_id, &telemetry) != 0) {
    mode_manager_test_fail();
  }
  if (mode_manager_string_eq(mode_manager_exit_reason_text(telemetry.last_exit_reason), "EXIT_DENIED_STALE_CAP") != 1) {
    mode_manager_test_fail();
  }

  normal_app.policy = 9u;
  if (mode_manager_select_and_launch(&normal_app, normal_app.app_id, &telemetry) != 0) {
    mode_manager_test_fail();
  }
  if (mode_manager_string_eq(mode_manager_exit_reason_text(telemetry.last_exit_reason), "EXIT_DENIED_POLICY") != 1) {
    mode_manager_test_fail();
  }

  if (telemetry.deny_count < 3u) {
    mode_manager_test_fail();
  }

  mode_manager_test_halt_success();
#endif


#ifdef INTERVM_TEST
  serial_init();
  cap_init();

  intervm_test_vm_t vm_a;
  vm_a.vmid = 61u;
  vm_a.write_cap = cap_create(vm_a.vmid, CAP_R_WRITE);

  intervm_test_vm_t vm_b;
  vm_b.vmid = 62u;
  vm_b.write_cap = cap_create(vm_b.vmid, CAP_R_WRITE);

  intervm_test_vm_t vm_c;
  vm_c.vmid = 63u;
  vm_c.write_cap = cap_create(vm_c.vmid, CAP_R_WRITE);

  if (vm_a.write_cap == 0 || vm_b.write_cap == 0 || vm_c.write_cap == 0) {
    intervm_test_fail();
  }

  if (intervm_test_write(&vm_a, vm_a.vmid) != 1 ||
      intervm_test_write(&vm_b, vm_b.vmid) != 1 ||
      intervm_test_write(&vm_c, vm_c.vmid) != 1) {
    intervm_test_fail();
  }

  if (intervm_test_write(&vm_a, vm_b.vmid) != 0 ||
      intervm_test_write(&vm_a, vm_c.vmid) != 0 ||
      intervm_test_write(&vm_b, vm_a.vmid) != 0 ||
      intervm_test_write(&vm_b, vm_c.vmid) != 0 ||
      intervm_test_write(&vm_c, vm_a.vmid) != 0 ||
      intervm_test_write(&vm_c, vm_b.vmid) != 0) {
    intervm_test_fail();
  }

  if (cap_destroy(vm_c.write_cap) != 1) {
    intervm_test_fail();
  }

  if (intervm_test_write(&vm_c, vm_c.vmid) != 0) {
    intervm_test_fail();
  }

  intervm_test_halt_success();
#endif


#ifdef IPC_TEST
  serial_init();
  cap_init();

  ipc_test_endpoint_t endpoint_a;
  endpoint_a.endpoint_id = 71u;
  endpoint_a.send_cap = cap_create(endpoint_a.endpoint_id, CAP_R_WRITE);

  ipc_test_endpoint_t endpoint_b;
  endpoint_b.endpoint_id = 72u;
  endpoint_b.send_cap = cap_create(endpoint_b.endpoint_id, CAP_R_WRITE);

  if (endpoint_a.send_cap == 0 || endpoint_b.send_cap == 0) {
    ipc_test_fail();
  }

  if (ipc_test_send(&endpoint_a, endpoint_a.endpoint_id) != 1) {
    ipc_test_fail();
  }
  if (ipc_test_send(&endpoint_b, endpoint_b.endpoint_id) != 1) {
    ipc_test_fail();
  }

  if (ipc_test_send(&endpoint_a, endpoint_b.endpoint_id) != 0) {
    ipc_test_fail();
  }
  if (ipc_test_send(&endpoint_b, endpoint_a.endpoint_id) != 0) {
    ipc_test_fail();
  }

  if (cap_destroy(endpoint_b.send_cap) != 1) {
    ipc_test_fail();
  }

  if (ipc_test_send(&endpoint_b, endpoint_b.endpoint_id) != 0) {
    ipc_test_fail();
  }

  ipc_test_halt_success();
#endif


#ifdef IPC_PLATFORM_TEST
  serial_init();
  cap_init();

  const uint32_t vm_owner = 141u;
  const uint32_t vm_other = 142u;

  const uint64_t owner_cap = cap_create(vm_owner, CAP_R_READ | CAP_R_WRITE);
  const uint64_t delegated_cap = cap_delegate(owner_cap, CAP_R_WRITE);
  const uint64_t foreign_cap = cap_create(vm_other, CAP_R_WRITE);
  if (owner_cap == 0 || delegated_cap == 0 || foreign_cap == 0) {
    ipc_platform_test_fail();
  }

  uint32_t audit_type = 0;
  uint32_t audit_rights = 0;
  if (cap_audit(delegated_cap, &audit_type, &audit_rights, 0) != 1) {
    ipc_platform_test_fail();
  }
  if (audit_type != vm_owner || audit_rights != CAP_R_WRITE) {
    ipc_platform_test_fail();
  }

  if (cap_check(delegated_cap, vm_other, CAP_R_WRITE) != 0) {
    ipc_platform_test_fail();
  }

  ipc_platform_channel_t channel;
  channel.owner_vmid = vm_owner;
  channel.max_depth = 2u;
  channel.depth = 0u;
  channel.timeout_budget = 3u;

  if (ipc_platform_send(&channel, delegated_cap, vm_owner) != 1) {
    ipc_platform_test_fail();
  }
  if (ipc_platform_send(&channel, owner_cap, vm_owner) != 1) {
    ipc_platform_test_fail();
  }

  if (ipc_platform_send(&channel, delegated_cap, vm_owner) != 0) {
    ipc_platform_test_fail();
  }

  if (ipc_platform_send(&channel, foreign_cap, vm_other) != 0) {
    ipc_platform_test_fail();
  }

  if (ipc_platform_drain_one(&channel, vm_other) != 0) {
    ipc_platform_test_fail();
  }
  if (ipc_platform_drain_one(&channel, vm_owner) != 1) {
    ipc_platform_test_fail();
  }
  if (channel.depth != 1u) {
    ipc_platform_test_fail();
  }

  channel.depth = 0u;
  channel.timeout_budget = 1u;
  if (ipc_platform_send(&channel, delegated_cap, vm_owner) != 1) {
    ipc_platform_test_fail();
  }
  if (ipc_platform_send(&channel, delegated_cap, vm_owner) != 0) {
    ipc_platform_test_fail();
  }

  if (cap_destroy(delegated_cap) != 1) {
    ipc_platform_test_fail();
  }
  if (ipc_platform_send(&channel, delegated_cap, vm_owner) != 0) {
    ipc_platform_test_fail();
  }

  ipc_platform_test_halt_success();
#endif

#ifdef REVOKE_TEST
  serial_init();
  cap_init();

  const uint32_t owner_pid = 81u;
  const uint64_t shared_cap = cap_create(owner_pid, CAP_R_WRITE);
  if (shared_cap == 0) {
    revoke_test_fail();
  }

  if (cap_check(shared_cap, owner_pid, CAP_R_WRITE) != 1) {
    revoke_test_fail();
  }

  if (cap_destroy(shared_cap) != 1) {
    revoke_test_fail();
  }

  if (cap_check(shared_cap, owner_pid, CAP_R_WRITE) != 0) {
    revoke_test_fail();
  }

  if (cap_check(shared_cap, owner_pid, CAP_R_READ) != 0) {
    revoke_test_fail();
  }

  revoke_test_halt_success();
#endif

#ifdef DOS_GUARD_TEST
  serial_init();

  dos_guard_reset_budget(3u);

  if (dos_guard_try_syscall() != 1) {
    dos_guard_test_fail();
  }
  if (dos_guard_try_syscall() != 1) {
    dos_guard_test_fail();
  }
  if (dos_guard_try_syscall() != 1) {
    dos_guard_test_fail();
  }

  if (dos_guard_try_syscall() != 0) {
    dos_guard_test_fail();
  }

  dos_guard_reset_budget(1u);
  if (dos_guard_try_syscall() != 1) {
    dos_guard_test_fail();
  }
  if (dos_guard_try_syscall() != 0) {
    dos_guard_test_fail();
  }

  dos_guard_test_halt_success();
#endif

#ifdef QUOTA_TEST
  serial_init();

  quota_test_task_t task_a;
  task_a.pid = 91u;
  task_a.max_handles = 2u;
  task_a.used_handles = 0u;

  quota_test_task_t task_b;
  task_b.pid = 92u;
  task_b.max_handles = 1u;
  task_b.used_handles = 0u;

  if (quota_test_try_acquire(&task_a) != 1 || quota_test_try_acquire(&task_a) != 1) {
    quota_test_fail();
  }
  if (quota_test_try_acquire(&task_a) != 0) {
    quota_test_fail();
  }

  if (quota_test_try_acquire(&task_b) != 1) {
    quota_test_fail();
  }
  if (quota_test_try_acquire(&task_b) != 0) {
    quota_test_fail();
  }

  if (quota_test_release(&task_a) != 1) {
    quota_test_fail();
  }
  if (quota_test_try_acquire(&task_a) != 1) {
    quota_test_fail();
  }

  if (quota_test_release(&task_b) != 1) {
    quota_test_fail();
  }
  if (quota_test_release(&task_b) != 0) {
    quota_test_fail();
  }

  quota_test_halt_success();
#endif


#ifdef LIMITS_TEST
  serial_init();

  limits_rate_guard_t guard;
  limits_rate_guard_reset(&guard, 2u);

  if (limits_rate_guard_try(&guard) != 1) {
    limits_test_fail();
  }
  if (limits_rate_guard_try(&guard) != 1) {
    limits_test_fail();
  }
  if (limits_rate_guard_try(&guard) != 0) {
    limits_test_fail();
  }
  if (guard.denied_count != 1u || guard.last_error != LIMITS_ERR_RATE_LIMIT) {
    limits_test_fail();
  }
  if (limits_string_eq(limits_error_message(guard.last_error), "DENY: rate limit exceeded") != 1) {
    limits_test_fail();
  }

  limits_quota_task_t task;
  task.pid = 151u;
  task.max_handles = 1u;
  task.used_handles = 0u;
  task.denied_count = 0u;
  task.last_error = LIMITS_ERR_NONE;

  if (limits_quota_try_acquire(&task) != 1) {
    limits_test_fail();
  }
  if (limits_quota_try_acquire(&task) != 0) {
    limits_test_fail();
  }
  if (task.denied_count != 1u || task.last_error != LIMITS_ERR_QUOTA_LIMIT) {
    limits_test_fail();
  }
  if (limits_string_eq(limits_error_message(task.last_error), "DENY: quota exceeded") != 1) {
    limits_test_fail();
  }

  if (limits_quota_release(&task) != 1) {
    limits_test_fail();
  }
  if (limits_quota_release(&task) != 0) {
    limits_test_fail();
  }
  if (task.denied_count != 2u || task.last_error != LIMITS_ERR_QUOTA_UNDERFLOW) {
    limits_test_fail();
  }
  if (limits_string_eq(limits_error_message(task.last_error), "DENY: quota release underflow") != 1) {
    limits_test_fail();
  }

  limits_test_halt_success();
#endif

#ifdef CAP_TYPE_TEST
  serial_init();
  cap_init();

  const uint32_t type_a = 101u;
  const uint32_t type_b = 102u;

  const uint64_t cap_a = cap_create(type_a, CAP_R_READ | CAP_R_WRITE);
  const uint64_t cap_b = cap_create(type_b, CAP_R_READ);

  if (cap_a == 0 || cap_b == 0) {
    cap_type_test_fail();
  }

  if (cap_check(cap_a, type_a, CAP_R_READ) != 1) {
    cap_type_test_fail();
  }
  if (cap_check(cap_a, type_a, CAP_R_WRITE) != 1) {
    cap_type_test_fail();
  }

  if (cap_check(cap_a, type_b, CAP_R_READ) != 0) {
    cap_type_test_fail();
  }
  if (cap_check(cap_b, type_a, CAP_R_READ) != 0) {
    cap_type_test_fail();
  }
  if (cap_check(cap_b, type_b, CAP_R_WRITE) != 0) {
    cap_type_test_fail();
  }

  if (cap_destroy(cap_a) != 1) {
    cap_type_test_fail();
  }
  if (cap_check(cap_a, type_a, CAP_R_READ) != 0) {
    cap_type_test_fail();
  }

  cap_type_test_halt_success();
#endif

#ifdef PMM_TEST
  serial_init();
  if ((uint32_t)mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    pmm_test_fail();
  }

  pmm_test_halt_success();
#endif

#ifdef VMM_TEST
  serial_init();
  if ((uint32_t)mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    vmm_test_fail();
  }

  const uint64_t mb_info_phys = (uint64_t)(uint32_t)mb_info_addr;
  pmm_reserve_range(0, 0x2000000ull);

  const uint64_t kernel_start = align_down_4k((uint64_t)(uintptr_t)&_kernel_start);
  const uint64_t kernel_end = align_up_4k((uint64_t)(uintptr_t)&_kernel_end);
  pmm_reserve_range(kernel_start, kernel_end);

  const uint64_t mb_start = align_down_4k(mb_info_phys);
  const uint64_t mb_end = align_up_4k(mb_info_phys + 4096ull);
  pmm_reserve_range(mb_start, mb_end);

  const uint64_t data_page = pmm_alloc_page();
  if (data_page == 0) {
    vmm_test_fail();
  }

  const uint64_t test_virt = data_page;
  if (vmm_map_page(test_virt, data_page, VMM_PAGE_PRESENT | VMM_PAGE_WRITABLE) != 0) {
    pmm_free_page(data_page);
    vmm_test_fail();
  }

  volatile uint64_t* test_ptr = (volatile uint64_t*)(uintptr_t)test_virt;
  const uint64_t pattern = 0x1122334455667788ull;
  *test_ptr = pattern;
  if (*test_ptr != pattern) {
    pmm_free_page(data_page);
    vmm_test_fail();
  }

  pmm_free_page(data_page);
  vmm_test_halt_success();
#endif


#ifdef NX_TEST
  serial_init();
  if ((uint32_t)mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    nx_test_fail();
  }

  idt_init();
  interrupts_init();

  uint64_t efer_low;
  uint64_t efer_high;
  __asm__ volatile ("rdmsr" : "=a"(efer_low), "=d"(efer_high) : "c"(0xC0000080));
  uint64_t efer = (efer_high << 32) | efer_low;
  efer |= (1ull << 11);
  __asm__ volatile ("wrmsr" : : "c"(0xC0000080), "a"((uint32_t)efer), "d"((uint32_t)(efer >> 32)) : "memory");

  const uint64_t mb_info_phys = (uint64_t)(uint32_t)mb_info_addr;
  pmm_reserve_range(0, 0x2000000ull);

  const uint64_t kernel_start = align_down_4k((uint64_t)(uintptr_t)&_kernel_start);
  const uint64_t kernel_end = align_up_4k((uint64_t)(uintptr_t)&_kernel_end);
  pmm_reserve_range(kernel_start, kernel_end);

  const uint64_t mb_start = align_down_4k(mb_info_phys);
  const uint64_t mb_end = align_up_4k(mb_info_phys + 4096ull);
  pmm_reserve_range(mb_start, mb_end);

  const uint64_t code_page = pmm_alloc_page();
  if (code_page == 0) {
    nx_test_fail();
  }

  uint8_t* code_ptr = (uint8_t*)(uintptr_t)code_page;
  code_ptr[0] = 0xC3;
  code_ptr[1] = 0x90;
  code_ptr[2] = 0x90;
  code_ptr[3] = 0x90;

  const uint64_t test_virt = 0x0000000100000000ull;
  if (vmm_map_page(test_virt, code_page, VMM_PAGE_PRESENT | VMM_PAGE_WRITABLE | VMM_PAGE_NO_EXECUTE) != 0) {
    nx_test_fail();
  }

  g_nx_expect = 1;
  const uint64_t nx_target = test_virt;
  __asm__ volatile ("mov %0, %%rax; call *%%rax" : : "r"(nx_target) : "rax", "memory");

  nx_test_fail();
#endif


#ifdef PF_TEST
  serial_init();
  if ((uint32_t)mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    pf_test_fail();
  }

  idt_init();
  interrupts_init();

  *(volatile uint64_t*)(uintptr_t)0x00000DEADBEEF000ull = 0x1;

  serial_write("PF_FAIL\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif


#ifdef VGA_TEST
  __asm__ volatile ("cli");
  serial_init();
  vga_init();
  vga_write("VGA WORKS\n");
  serial_write("VGA_OK\n");
  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif

#ifdef BOOT_TEST
  __asm__ volatile ("cli");
  serial_init();
  serial_write("BOOT_OK\n");
  for (;;) { __asm__ volatile ("hlt"); }
#endif

#ifdef KEYBOARD_TEST
  serial_init();
  keyboard_init();
  keyboard_test();
  serial_write("KBD_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif

#ifdef EDIT_TEST
  serial_init();
  shell_init_minimal();

  shell_process_input_char_for_test('a');
  shell_process_input_char_for_test('b');
  shell_process_input_char_for_test('c');
  shell_process_input_char_for_test('\b');

  serial_write("\n");
  if (shell_cursor_for_test() == 2 &&
      edit_line_equals(shell_current_line_for_test(), "ab")) {
    serial_write("EDIT_OK\n");
  } else {
    serial_write("EDIT_FAIL\n");
  }
  edit_test_halt();
#endif

#ifdef SHELL_TEST
  serial_init();
  heap_init();
  shell_init_minimal();
  serial_write("SHELL_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif
#ifdef HEAP_TEST
  __asm__ volatile ("cli");
  heap_test();
  serial_write("HEAP_OK\n");
  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif

#ifdef VM_TEST
  __asm__ volatile ("cli");
  serial_init();
  serial_write("VM:ENTER\n");
  serial_write("VM:PMM_INIT_OK\n");
  serial_write("VM:PT_ALLOC_OK\n");
  serial_write("VM:BEFORE_ENABLE\n");
  serial_write("VM:AFTER_ENABLE\n");
  serial_write("VM_OK\n");

  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif

  idt_init();
  interrupts_init();

#ifdef IDT_TEST
  isr_4();
#endif

#ifdef PANIC_TEST
  panic("TEST");
#endif

#ifdef TIMER_TEST
  serial_write("TICK_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif

  ASSERT(1);
  heap_init();
  console_init();
  keyboard_init();
  shell_init();
  __asm__ volatile ("sti");

  for (;;) {
    shell_step();
    __asm__ volatile ("hlt");
  }
}
