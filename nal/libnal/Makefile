SRC_FILES = syscall.s
DEPS = $(addprefix build/, $(addsuffix .d, $(SRC_FILES)))
OBJ = $(addprefix build/, $(addsuffix .o, $(SRC_FILES)))
SRC = $(addprefix src/, $(SRC_FILES))
OUT = $(NAL_DIR)/nal/lib/libnal.a

include ../toolchain.mk

.PHONY: all clean

all: $(OUT)

$(OUT): $(OBJ)
	@mkdir -p $(dir $(OUT))
	ar -crs $@ $(OBJ)

-include $(DEPS)

build/%.c.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP $< -o $@

build/%.s.o: src/%.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) -MD -MP -MF build/$*.s.d $< -o $@

clean:
	rm -rf build/
