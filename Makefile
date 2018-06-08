arch ?= x86_64
kernel := build/kernel-$(arch).bin
iso := build/os-$(arch).iso

BIN_PREFIX = /usr/local/x86_64/bin
CC = $(BIN_PREFIX)/x86_64-elf-gcc
LD = $(BIN_PREFIX)/x86_64-elf-ld
NASM = /usr/local/Cellar/nasm/2.13.03/bin/nasm
GDB = $(BIN_PREFIX)/x86_64-elf-gdb
QEMU = /usr/local/bin/qemu-system-x86_64
GRUB = $(BIN_PREFIX)/grub-mkrescue

CFLAGS = -g -ffreestanding
ASFLAGS = -f elf64
QEMUFLAGS = -m 4096M

linker := src/arch/$(arch)/linker.ld
grub_cfg := src/arch/$(arch)/grub.cfg
as_src := $(wildcard src/arch/$(arch)/*.asm)
as_obj := $(patsubst src/arch/$(arch)/%.asm, \
    build/arch/$(arch)/%.o, $(as_src))

subdirs := $(wildcard src/*/)
c_src := $(wildcard $(addsuffix *.c,$(subdirs)))
c_obj := $(patsubst src/%.c, build/%.o,$(c_src))

.PHONY: all clean run iso

all: $(kernel)

clean:
	@rm -r build

run: $(iso)
	$(QEMU) $(QEMUFLAGS) -cdrom $(iso)

iso: $(iso)

$(iso): $(kernel) $(grub_cfg)
	@mkdir -p build/isofiles/boot/grub
	@cp $(kernel) build/isofiles/boot/kernel.bin
	@cp $(grub_cfg) build/isofiles/boot/grub
	$(GRUB) -o $(iso) build/isofiles 2> /dev/null
	@rm -r build/isofiles

$(kernel): $(as_obj) $(linker) $(c_obj)
	$(LD) -n -T $(linker) -o $(kernel) $(as_obj) $(c_obj)

build/%.o: src/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) $< -c -o $@

build/arch/$(arch)/%.o: src/arch/$(arch)/%.asm
	@mkdir -p $(shell dirname $@)
	$(NASM) $(ASFLAGS) $< -o $@
