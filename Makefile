C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c libc/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h libc/*.h)

OBJ = ${C_SOURCES:.c=.o}

CC = /usr/local/386gcc/bin/i386-elf-gcc
LD = /usr/local/386gcc/bin/i386-elf-ld
AS = /usr/local/386gcc/bin/i386-elf-as

GDB = /usr/local/386gcc/bin/i386-elf-gdb
QEMU = /usr/local/bin/qemu-system-i386

CFLAGS = -g -ffreestanding -Wno-int-conversion -m32 -Wall -Wextra -Werror

wos.bin: boot.o ${OBJ} interrupt.o
	${CC} -T linker.ld -o wos.bin -ffreestanding -O2 boot/boot.o ${OBJ} cpu/interrupt.o -nostdlib

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

boot.o:
	${AS} boot/boot.s -o boot/$@

debug: wos.bin
	${QEMU} -s -kernel $< -d guest_errors &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file $<"

interrupt.o:
	 nasm cpu/interrupt.asm -f elf -o cpu/$@

default: wos.bin
	qemu-system-i386 -kernel $<

clean: 
	rm -rf *.o *.bin
	rm -rf boot/*.o kernel/*.o drivers/*.o cpu/*.o libc/*.o