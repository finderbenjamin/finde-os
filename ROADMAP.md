1) BOOT_OK über Serial (QEMU, GRUB, Multiboot2)
2) PANIC(msg) + ASSERT(x) -> Serial "PANIC: ..."
3) IDT init -> "IDT_OK"
4) PIT Timer IRQ ticks zählen -> "TICK_OK"
5) Simple Heap (bump allocator) + Test -> "HEAP_OK"
