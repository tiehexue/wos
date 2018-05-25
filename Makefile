C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c libc/*.c)
TEST_SOURCES = $(wildcard test/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h libc/*.h test/*.h)

OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o}
TEST_OBJ = ${TEST_SOURCES:.c=.o}

CC = /usr/local/386gcc/bin/i386-elf-gcc
LD = /usr/local/386gcc/bin/i386-elf-ld

GDB = /usr/local/386gcc/bin/i386-elf-gdb
QEMU = /usr/local/bin/qemu-system-i386
NASM = /usr/local/Cellar/nasm/2.13.03/bin/nasm

CFLAGS = -g -ffreestanding -Wno-int-conversion -m32 -Wall -Wextra -Werror

all: run

os-image.bin: boot/bootsect.bin kernel.bin test.bin
	cat $^ > $@

kernel.bin: boot/kernel_entry.o ${OBJ} ${TEST_OBJ}
	${LD} -o $@ -Ttext 0x1000 $^ --oformat binary

test.bin: ${TEST_OBJ}
	${LD} -o $@ -Ttext 0x400000 $^ --oformat binary	

kernel.elf: boot/kernel_entry.o ${OBJ}
	${LD} -o $@ -Ttext 0x1000 $^

debug: os-image.bin kernel.elf
	${QEMU} -s -fda os-image.bin -d guest_errors,int &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	${NASM} $< -f elf -o $@

%.bin: %.asm
	${NASM} $< -f bin -o $@

run: os-image.bin
	${QEMU} -fda $<

clean: 
	rm -rf *.bin *.o *.dis *.elf
	rm -rf boot/*.bin kernel/*.o kernel_entry/*.o drivers/*.o boot/*.o, cpu/*.o, libc/*.o
