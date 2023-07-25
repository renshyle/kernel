AS = nasm
CC = clang
LD = ld.lld
ASFLAGS = -f elf64 -Werror
CFLAGS = -c -ffreestanding -fno-builtin -nostdlib -Wall -Wextra \
		 -fno-stack-protector -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
		 --target=x86_64-elf
LDFLAGS = -T $(NAL_DIR)/nal/linker.ld -melf_x86_64 -nostdlib -static
