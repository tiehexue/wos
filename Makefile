C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c libc/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h libc/*.h)

OBJ = ${C_SOURCES:.c=.o}

CC = /usr/local/386gcc/bin/i386-elf-gcc
LD = /usr/local/386gcc/bin/i386-elf-ld
NASM = /usr/local/Cellar/nasm/2.13.03/bin/nasm
GDB = /usr/local/386gcc/bin/i386-elf-gdb

CFLAGS = -g -ffreestanding -Wno-int-conversion -m32 -Wall -Wextra -Werror \
	-Wno-error=unused-function -Wno-error=unused-variable -Wno-error=unused-parameter
ASFLAGS = -f elf

QEMU = /usr/local/bin/qemu-system-i386

default: wos.bin
	${QEMU} -kernel $<

wos.bin: ${OBJ} boot/boot.o cpu/interrupt.o cpu/gdt_flush.o
	${CC} -T linker.ld -o wos.bin -ffreestanding -O2 boot/boot.o cpu/interrupt.o cpu/gdt_flush.o ${OBJ} -nostdlib

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	${NASM} ${ASFLAGS} $<

debug: wos.bin
	${QEMU} -s -kernel $< -d guest_errors &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file $<"

clean: 
	rm -rf *.o *.bin
	rm -rf boot/*.o kernel/*.o drivers/*.o cpu/*.o libc/*.o