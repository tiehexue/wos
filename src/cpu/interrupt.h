#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>
#include "../kernel/types.h"

typedef struct {
    uint64_t rbp;
    uint64_t error_no;
    uint64_t error_code;
    uint64_t rip;
    uint64_t rflags;
    uint64_t cs;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed)) isr_regs;

typedef struct {
    sse_128 xmm_registers[16];
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rbp;
    uint64_t code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ds;
} __attribute__((packed)) irq_regs;

void interrupt_init();

bool register_irq_handler(size_t irq, void (*handler)(irq_regs*, void*), void* data);

bool unregister_irq_handler(size_t irq, void (*handler)(irq_regs*, void*));

#endif
