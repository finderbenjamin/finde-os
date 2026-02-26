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

all: $(BUILD)/os.iso

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.o: boot/boot.s | $(BUILD)
	$(AS) $(ASFLAGS) boot/boot.s -o $(BUILD)/boot.o

$(BUILD)/kernel.o: kernel/kernel.c | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/kernel.c -o $(BUILD)/kernel.o

$(BUILD)/kernel.elf: $(BUILD)/boot.o $(BUILD)/kernel.o linker.ld
	$(LD) $(LDFLAGS) $(BUILD)/boot.o $(BUILD)/kernel.o -o $(BUILD)/kernel.elf

$(BUILD)/os.iso: $(BUILD)/kernel.elf boot/grub/grub.cfg
	rm -rf $(ISO_DIR)
	mkdir -p $(GRUB_DIR)
	cp $(BUILD)/kernel.elf $(BOOT_DIR)/kernel.elf
	cp boot/grub/grub.cfg $(GRUB_DIR)/grub.cfg
	grub-mkrescue -o $(BUILD)/os.iso $(ISO_DIR) >/dev/null 2>&1

clean:
	rm -rf $(BUILD) log.txt

.PHONY: all clean