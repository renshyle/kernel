SRC_FILES = main.c
DEPS = $(addprefix build/, $(addsuffix .d, $(SRC_FILES)))
OBJ = $(addprefix build/, $(addsuffix .o, $(SRC_FILES)))
SRC = $(addprefix src/, $(SRC_FILES))
OUT = $(NAL_DIR)/iso/nal/helloworld

include ../toolchain.mk

.PHONY: all clean

all: $(OUT)

$(OUT): $(OBJ)
	@mkdir -p $(dir $(OUT))
	$(LD) $(LDFLAGS) $(NAL_DIR)/nal/lib/crt0.o $(OBJ) -o $@

-include $(DEPS)

build/%.c.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP $< -o $@

clean:
	rm -rf build/
