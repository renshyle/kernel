SRC_FILES = debug.c entry.c entry.s gdt.c gdt.s int.c int.s lib.c panic.c panic.s phys.c pic.c pit.c task.c task.s virt.c virt.s
DEPS = $(addprefix build/, $(addsuffix .d, $(SRC_FILES)))
OBJ = $(addprefix build/, $(addsuffix .o, $(SRC_FILES)))
SRC = $(addprefix src/, $(SRC_FILES))
KERNEL_ELF = build/nal.elf

AS = nasm
CC = clang
LD = ld.lld
ASFLAGS = -f elf64 -Werror
CFLAGS = -c -ffreestanding -fno-builtin -nostdlib -mno-red-zone -Wall -Wextra \
		 -fno-stack-protector -fno-lto -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
		 --target=x86_64-elf -mcmodel=kernel
LDFLAGS = -T linker.ld -melf_x86_64 -nostdlib -static -z text \
		  -z max-page-size=0x1000

ifdef DEBUG
	CFLAGS += -DDEBUG=1 -g
	LDFLAGS += -g
endif

.PHONY: clean run

all: $(KERNEL_ELF)

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

clean:
	rm -rf build/
