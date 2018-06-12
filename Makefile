arch ?= x86_64
kernel := build/kernel-$(arch).bin
iso := build/os-$(arch).iso

BIN_PREFIX = /usr/local/x86_64/bin
CC = $(BIN_PREFIX)/x86_64-elf-gcc
CXX = $(BIN_PREFIX)/x86_64-elf-g++
LD = $(BIN_PREFIX)/x86_64-elf-ld
GDB = $(BIN_PREFIX)/x86_64-elf-gdb
GRUB = $(BIN_PREFIX)/grub-mkrescue

NASM = /usr/local/Cellar/nasm/2.13.03/bin/nasm
QEMU = /usr/local/bin/qemu-system-x86_64

CFLAGS = -masm=intel -nostdlib -g -Os -fno-stack-protector -fno-exceptions \
	-funsigned-char -ffreestanding -fomit-frame-pointer -mno-red-zone -mno-3dnow \
	-mno-mmx -fno-asynchronous-unwind-tables -Isrc/tstl/include/
CXXFLAGS = $(CFLAGS) -std=c++11 -fno-rtti
ASFLAGS = -f elf64
QEMUFLAGS = -m 4096M

linker := src/arch/$(arch)/linker.ld
grub_cfg := src/arch/$(arch)/grub.cfg
boot_src := $(wildcard src/arch/$(arch)/*.asm)
boot_obj := $(patsubst src/arch/$(arch)/%.asm, \
    build/arch/$(arch)/%.o, $(boot_src))

subdirs := $(wildcard src/*/)
as_subdirs := $(filter-out src/arch, $(subdirs))
as_src := $(wildcard $(addsuffix *.asm,$(as_subdirs)))
as_obj := $(patsubst src/%.asm, build/%.o,$(as_src))

c_src := $(wildcard $(addsuffix *.c,$(subdirs)))
c_obj := $(patsubst src/%.c, build/%.o,$(c_src))

cpp_src := $(wildcard $(addsuffix *.cpp,$(subdirs)))
cpp_obj := $(patsubst src/%.cpp, build/%.o,$(cpp_src))

.PHONY: all clean run iso

all: $(kernel)

clean:
	@rm -r build

run: $(iso)
	$(QEMU) $(QEMUFLAGS) -cdrom $(iso)

debug: $(iso)
	$(QEMU) -s $(QEMUFLAGS) -cdrom $(iso) &
	${GDB}  -ex "target remote localhost:1234" -ex "symbol-file $(kernel)"

iso: $(iso)

$(iso): $(kernel) $(grub_cfg)
	@mkdir -p build/isofiles/boot/grub
	@cp $(kernel) build/isofiles/boot/kernel.bin
	@cp $(grub_cfg) build/isofiles/boot/grub
	$(GRUB) -o $(iso) build/isofiles 2> /dev/null
	@rm -r build/isofiles

$(kernel): $(boot_obj) $(linker) $(c_obj) $(cpp_obj) $(as_obj)
	$(LD) -n -T $(linker) -o $(kernel) $^

build/%.o: src/%.asm
	@mkdir -p $(shell dirname $@)
	$(NASM) $(ASFLAGS) $< -o $@

build/%.o: src/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) $< -c -o $@

build/%.o: src/%.cpp
	@mkdir -p $(shell dirname $@)
	$(CXX) $(CXXFLAGS) $< -c -o $@

build/arch/$(arch)/%.o: src/arch/$(arch)/%.asm
	@mkdir -p $(shell dirname $@)
	$(NASM) $(ASFLAGS) $< -o $@

initrd.img:
	cc -o tools/generate_initrd tools/generate_initrd.c
	./tools/generate_initrd ./tools/t1.txt ./tools/t2.txt

