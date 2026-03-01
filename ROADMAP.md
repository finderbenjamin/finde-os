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
