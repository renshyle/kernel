SRC_FILES = debug.c entry.c gdt.c gdt.s int.c int.s panic.c phys.c pic.c
DEPS = $(addprefix build/, $(addsuffix .d, $(SRC_FILES)))
OBJ = $(addprefix build/, $(addsuffix .o, $(SRC_FILES)))
SRC = $(addprefix src/, $(SRC_FILES))
ISO_ROOT = build/iso
KERNEL_ELF = $(ISO_ROOT)/kernel.elf
OUT = build/kernel.iso

LIMINE_BINS=$(ISO_ROOT)/limine-bios.sys $(ISO_ROOT)/limine-bios-cd.bin $(ISO_ROOT)/limine-uefi-cd.bin $(ISO_ROOT)/EFI/BOOT/BOOTX64.EFI $(ISO_ROOT)/EFI/BOOT/BOOTIA32.EFI
LIMINE_CFG=limine.cfg

AS = nasm
CC = clang
LD = ld.lld
ASFLAGS = -f elf64 -Werror
CFLAGS = -c -ffreestanding -fno-builtin -nostdlib -mno-red-zone -Wall -Wextra \
		 -fno-stack-protector -fno-lto -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
		 --target=x86_64-elf -mcmodel=large
LDFLAGS = -T linker.ld -melf_x86_64 -nostdlib -static -z text \
		  -z max-page-size=0x1000

QEMUFLAGS = -serial stdio

ifdef DEBUG
	CFLAGS += -DDEBUG=1 -g
	QEMUFLAGS += -d cpu_reset
endif

.PHONY: clean run

all: $(OUT)

$(OUT): $(KERNEL_ELF) $(LIMINE_BINS) $(ISO_ROOT)/$(LIMINE_CFG)
	xorriso -as mkisofs -b limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label $(ISO_ROOT) -o $(OUT)

$(ISO_ROOT)/$(LIMINE_CFG): $(LIMINE_CFG)
	@mkdir -p $(ISO_ROOT)
	cp $< $@

$(ISO_ROOT)/%: /usr/share/limine/%
	@mkdir -p $(ISO_ROOT)
	cp $< $@

$(ISO_ROOT)/EFI/BOOT/%: /usr/share/limine/%
	@mkdir -p $(ISO_ROOT)/EFI/BOOT
	cp $< $@

$(KERNEL_ELF): $(OBJ) linker.ld
	@mkdir -p $(@D)
	$(LD) $(LDFLAGS) $(OBJ) -o $@

-include $(DEPS)

build/%.s.o: src/%.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) -MD -MP -MF build/$*.s.d $< -o $@

build/%.c.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP $< -o $@

run: $(OUT)
	qemu-system-x86_64 $(QEMUFLAGS) -drive format=raw,media=cdrom,file=$(OUT)

clean:
	rm -rf build/
