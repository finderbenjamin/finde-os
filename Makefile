CC=clang
LD=ld.lld
AS=clang
CFLAGS=-ffreestanding -fno-stack-protector -fno-pic -mno-red-zone -m64 -O2 -Wall -Wextra
ASFLAGS=-c -ffreestanding -m64
LDFLAGS=-T linker.ld -nostdlib

BUILD=build
ISO_DIR=$(BUILD)/isodir
BOOT_DIR=$(ISO_DIR)/boot
GRUB_DIR=$(BOOT_DIR)/grub
PANIC_TEST?=0
IDT_TEST?=0
TIMER_TEST?=0
HEAP_TEST?=0
SHELL_TEST?=0
KEYBOARD_TEST?=0
VM_TEST?=0

ifeq ($(PANIC_TEST),1)
CFLAGS += -DPANIC_TEST
endif

ifeq ($(IDT_TEST),1)
CFLAGS += -DIDT_TEST
endif

ifeq ($(TIMER_TEST),1)
CFLAGS += -DTIMER_TEST
endif

ifeq ($(HEAP_TEST),1)
CFLAGS += -DHEAP_TEST
endif

ifeq ($(SHELL_TEST),1)
CFLAGS += -DSHELL_TEST
endif

ifeq ($(KEYBOARD_TEST),1)
CFLAGS += -DKEYBOARD_TEST
endif

ifeq ($(VM_TEST),1)
CFLAGS += -DVM_TEST
endif

all: $(BUILD)/finde-os.iso

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.o: boot/boot.s | $(BUILD)
	$(AS) $(ASFLAGS) boot/boot.s -o $(BUILD)/boot.o

$(BUILD)/isr.o: boot/isr.s | $(BUILD)
	$(AS) $(ASFLAGS) boot/isr.s -o $(BUILD)/isr.o

$(BUILD)/kernel.o: kernel/kernel.c kernel/console.h kernel/heap.h kernel/idt.h kernel/keyboard.h kernel/multiboot1.h kernel/multiboot2.h kernel/paging.h kernel/panic.h kernel/pmm.h kernel/serial.h kernel/shell.h kernel/vga.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/kernel.c -o $(BUILD)/kernel.o

$(BUILD)/panic.o: kernel/panic.c kernel/panic.h kernel/serial.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/panic.c -o $(BUILD)/panic.o

$(BUILD)/serial.o: kernel/serial.c kernel/serial.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/serial.c -o $(BUILD)/serial.o

$(BUILD)/idt.o: kernel/idt.c kernel/idt.h kernel/serial.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/idt.c -o $(BUILD)/idt.o

$(BUILD)/heap.o: kernel/heap.c kernel/heap.h kernel/panic.h kernel/serial.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/heap.c -o $(BUILD)/heap.o

$(BUILD)/shell.o: kernel/shell.c kernel/console.h kernel/shell.h kernel/heap.h kernel/idt.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/shell.c -o $(BUILD)/shell.o

$(BUILD)/vga.o: kernel/vga.c kernel/vga.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/vga.c -o $(BUILD)/vga.o

$(BUILD)/console.o: kernel/console.c kernel/console.h kernel/keyboard.h kernel/serial.h kernel/vga.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/console.c -o $(BUILD)/console.o

$(BUILD)/keyboard.o: kernel/keyboard.c kernel/keyboard.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/keyboard.c -o $(BUILD)/keyboard.o

$(BUILD)/pmm.o: kernel/pmm.c kernel/pmm.h kernel/multiboot1.h kernel/multiboot2.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/pmm.c -o $(BUILD)/pmm.o

$(BUILD)/paging.o: kernel/paging.c kernel/paging.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/paging.c -o $(BUILD)/paging.o

$(BUILD)/kernel.elf: $(BUILD)/boot.o $(BUILD)/isr.o $(BUILD)/kernel.o $(BUILD)/panic.o $(BUILD)/serial.o $(BUILD)/idt.o $(BUILD)/heap.o $(BUILD)/shell.o $(BUILD)/vga.o $(BUILD)/console.o $(BUILD)/keyboard.o $(BUILD)/pmm.o $(BUILD)/paging.o linker.ld
	$(LD) $(LDFLAGS) $(BUILD)/boot.o $(BUILD)/isr.o $(BUILD)/kernel.o $(BUILD)/panic.o $(BUILD)/serial.o $(BUILD)/idt.o $(BUILD)/heap.o $(BUILD)/shell.o $(BUILD)/vga.o $(BUILD)/console.o $(BUILD)/keyboard.o $(BUILD)/pmm.o $(BUILD)/paging.o -o $(BUILD)/kernel.elf

$(BUILD)/finde-os.iso: $(BUILD)/kernel.elf boot/grub/grub.cfg
	rm -rf $(ISO_DIR)
	mkdir -p $(GRUB_DIR)
	cp $(BUILD)/kernel.elf $(BOOT_DIR)/kernel.elf
	cp boot/grub/grub.cfg $(GRUB_DIR)/grub.cfg
	grub-mkrescue -o $(BUILD)/finde-os.iso $(ISO_DIR) >/dev/null 2>&1

clean:
	rm -rf $(BUILD) log.txt panic_log.txt idt_log.txt timer_log.txt heap_log.txt shell_log.txt keyboard_log.txt vm_log.txt

.PHONY: all clean
