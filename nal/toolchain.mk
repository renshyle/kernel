AS = nasm
CC = clang
LD = ld.lld
ASFLAGS = -f elf64 -Werror -i $(NAL_DIR)/nal/libnal/include
CFLAGS = -c -ffreestanding -fno-builtin -nostdlib -Wall -Wextra \
		 -fno-stack-protector -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
		 --target=x86_64-elf -I $(NAL_DIR)/nal/libnal/include
LDFLAGS = -T $(NAL_DIR)/nal/linker.ld -melf_x86_64 -nostdlib -static -L $(NAL_DIR)/nal/lib
