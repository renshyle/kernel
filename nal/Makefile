COMPONENTS = helloworld
COMPONENTSCLEAN = $(addsuffix .clean,$(COMPONENTS))

LIBDIR = $(CURDIR)/lib
OBJ = $(LIBDIR)/crt0.o

include toolchain.mk

.PHONY: all clean lib-clean $(COMPONENTS) $(COMPONENTSCLEAN)

all: $(OBJ) $(COMPONENTS)

$(COMPONENTS): $(LIBDIR)/crt0.o
	$(MAKE) -C $@

$(LIBDIR)/crt0.o: crt0.s
	@mkdir -p $(LIBDIR)
	$(AS) $(ASFLAGS) $< -o $@

clean: lib-clean $(COMPONENTSCLEAN)

lib-clean:
	rm -rf $(LIBDIR)

$(COMPONENTSCLEAN): %.clean:
	$(MAKE) -C $* clean
