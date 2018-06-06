// common.c -- Defines some global functions.
//             From JamesM's kernel development tutorials.

#include "common.h"
#include "../drivers/screen.h"

extern void panic(char *message, char *file, uint32_t line)
{
    // We encountered a massive problem and have to stop.
    asm volatile("cli"); // Disable interrupts.

    kprint("PANIC(");
    kprint(message);
    kprint(") at ");
    kprint(file);
    kprint(":");
    kprint_int(line);
    kprintln("");
    // Halt by going into an infinite loop.
    for(;;);
}

extern void panic_assert(char *file, uint32_t line, char *desc)
{
    // An assertion failed, and we have to panic.
    asm volatile("cli"); // Disable interrupts.

    kprint("ASSERTION-FAILED(");
    kprint(desc);
    kprint(") at ");
    kprint(file);
    kprint(":");
    kprint_int(line);
    kprintln("");
    // Halt by going into an infinite loop.
    for(;;);
}
