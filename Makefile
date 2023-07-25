COMPONENTS = kernel nal
COMPONENTSCLEAN = $(addsuffix .clean,$(COMPONENTS))
ISO_ROOT = iso
KERNEL_ELF = kernel/build/nal.elf
OUT = nal.iso

LIMINE_BINS=$(ISO_ROOT)/limine-bios.sys $(ISO_ROOT)/limine-bios-cd.bin $(ISO_ROOT)/limine-uefi-cd.bin $(ISO_ROOT)/EFI/BOOT/BOOTX64.EFI $(ISO_ROOT)/EFI/BOOT/BOOTIA32.EFI
LIMINE_CFG=limine.cfg

QEMUFLAGS = -serial stdio

NAL_DIR = $(CURDIR)
export NAL_DIR

ifndef LIMINE_DIR
	LIMINE_DIR = /usr/share/limine
endif

ifdef DEBUG
	QEMUFLAGS += -d cpu_reset
endif

.PHONY: all run clean iso-clean $(COMPONENTS) $(COMPONENTSCLEAN)

all: $(OUT)

$(OUT): $(COMPONENTS) $(ISO_ROOT)/$(LIMINE_CFG) $(LIMINE_BINS)
	@cp $(KERNEL_ELF) iso/
	xorriso -as mkisofs -b limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label $(ISO_ROOT) -o $(OUT)

$(COMPONENTS):
	$(MAKE) -C $@

$(ISO_ROOT)/$(LIMINE_CFG): $(LIMINE_CFG)
	@mkdir -p $(ISO_ROOT)
	cp $< $@

$(ISO_ROOT)/%: $(LIMINE_DIR)/%
	@mkdir -p $(ISO_ROOT)
	cp $< $@

$(ISO_ROOT)/EFI/BOOT/%: $(LIMINE_DIR)/%
	@mkdir -p $(ISO_ROOT)/EFI/BOOT
	cp $< $@

run: $(OUT)
	qemu-system-x86_64 $(QEMUFLAGS) -drive format=raw,media=cdrom,file=$(OUT)

clean: $(COMPONENTSCLEAN) iso-clean

iso-clean:
	rm -rf $(ISO_ROOT) $(OUT)

$(COMPONENTSCLEAN): %.clean:
	$(MAKE) -C $* clean
