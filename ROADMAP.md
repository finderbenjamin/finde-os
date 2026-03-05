1) BOOT_OK über Serial (QEMU, GRUB, Multiboot2)
2) PANIC(msg) + ASSERT(x) -> Serial "PANIC: ..."
3) IDT init -> "IDT_OK"
4) PIT Timer IRQ ticks zählen -> "TICK_OK"
5) Simple Heap (bump allocator) + Test -> "HEAP_OK"
6) Minimal shell init test -> "SHELL_OK"
7) Keyboard decoder test -> "KBD_OK"
8) Virtual memory stack test -> "VM_OK"
9) Physical memory manager test via multiboot2 map -> "PMM_OK"
10) VMM test -> "VMM_OK"
11) Page fault handler with CR2 + error code decode test -> "PF_OK"
12) NX (non-executable pages) deterministic test -> "NX_OK"
13) VGA text console deterministic check -> "VGA_OK"

14) Shell line editing with Backspace support test -> "EDIT_OK"

15) Capability primitives test -> "CAP_OK"

16) Capability enforcement gate test -> "CAP_ENFORCE_OK"

17) Syscall capability gate test -> "SYSCALL_OK"

18) Minimal task scheduler deterministic test -> "TASK_OK"

19) Task capability isolation test -> "TASK_CAP_OK"

20) Capability generation stale-handle guard test -> "CAP_GEN_OK"

21) Minimal user mode launch path test -> "USERMODE_OK"

22) Syscall deny path test -> "SYSCALL_DENY_OK"

23) Minimal user task capability write path test -> "USER_TASK_OK"

24) User task capability deny path test -> "USER_TASK_DENY_OK"

25) User-space driver isolation smoke test -> "DRV_ISO_OK"

26) Minimal MicroVM isolation smoke test -> "MICROVM_OK"

27) MicroVM capability boundary test -> "MICROVM_CAP_OK"


28) Inter-VM capability isolation test -> "INTERVM_OK"

29) IPC capability channel test -> "IPC_OK"

30) Capability revocation propagation test -> "REVOKE_OK"

31) Syscall rate guard test -> "DOS_GUARD_OK"

32) Process/task resource quota test -> "QUOTA_OK"

33) Capability type isolation matrix test -> "CAP_TYPE_OK"

34) Capability lifecycle create/delegate/revoke/audit test -> "CAP_LIFECYCLE_OK"

35) IPC + Inter-VM robustness (error paths, timeout budget, backpressure, ownership) test -> "IPC_PLATFORM_OK"

36) Resource/DoS telemetry + deny-reason clarity test -> "LIMITS_OK"

37) User-mode/driver path hardening (crash isolation + clear exit reasons) test -> "USERMODE_PATH_OK"

38) MicroVM golden security mode boundary + mode-switch reliability test -> "MICROVM_MODE_OK"

39) Unified mode manager (policy->sandbox/microvm path + clear deny reasons) test -> "MODE_MANAGER_OK"

40) CLI capability security introspection (cap list/show/check + unified deny reason + deterministic marker) test -> "CLI_SECURITY_OK"

41) CLI parser/validator/executor layering with deny-path validation test -> "CLI_LAYERS_OK"

42) CLI baseline command pipeline smoke test -> "CLI_BASE_OK"

43) CLI status command (sandbox/microvm visibility + deterministic marker) test -> "CLI_STATUS_OK"

44) CLI global help/onboarding UX + unknown-command suggestion test -> "CLI_HELP_OK"
