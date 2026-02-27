1) BOOT_OK über Serial (QEMU, GRUB, Multiboot2)
2) PANIC(msg) + ASSERT(x) -> Serial "PANIC: ..."
3) IDT init -> "IDT_OK"
4) PIT Timer IRQ ticks zählen -> "TICK_OK"
5) Simple Heap (bump allocator) + Test -> "HEAP_OK"
6) Minimal shell init test -> "SHELL_OK"
7) Keyboard decoder test -> "KBD_OK"
8) Virtual memory stack test -> "VM_OK"
9) Physical memory manager test via multiboot2 map -> "PMM_OK"
