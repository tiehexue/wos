C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c libc/*.c vfs/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h libc/*.h vfs/*.h)

OBJ = ${C_SOURCES:.c=.o}

CC = /usr/local/386gcc/bin/i386-elf-gcc
LD = /usr/local/386gcc/bin/i386-elf-ld
NASM = /usr/local/Cellar/nasm/2.13.03/bin/nasm
GDB = /usr/local/386gcc/bin/i386-elf-gdb
QEMU = /usr/local/bin/qemu-system-i386

CFLAGS = -g -ffreestanding -Wno-int-conversion -Wno-error=incompatible-pointer-types -m32 -Wall -Wextra -Werror -Wno-error=parentheses \
	-Wno-error=unused-function -Wno-error=sign-compare -Wno-error=unused-variable -Wno-error=unused-parameter
ASFLAGS = -f elf
QEMUFLAGS = -m 4096M -d guest_errors -initrd initrd.img

default: wos.bin
	${QEMU} -kernel $< ${QEMUFLAGS}

wos.bin: boot/boot.o cpu/interrupt.o cpu/gdt_flush.o cpu/process.o ${OBJ}
	${CC} -T linker.ld -o $@ -ffreestanding -O2 $^ -nostdlib

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	${NASM} ${ASFLAGS} $<

debug: wos.bin
	${QEMU} -s -kernel $< ${QEMUFLAGS} &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file $<"

initrd.img:
	cc -o tools/generate_initrd tools/generate_initrd.c
	./tools/generate_initrd ./tools/t1.txt ./tools/t2.txt

clean: 
	rm -rf *.o *.bin
	rm -rf boot/*.o kernel/*.o drivers/*.o cpu/*.o libc/*.o vfs/*.o tools/generate_initrd