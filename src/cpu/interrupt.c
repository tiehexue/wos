#include "interrupt.h"

#include "ports.h"
#include "../drivers/screen.h"

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

typedef struct {
    uint8_t type    : 4;
    uint8_t zero    : 1;
    uint8_t dpl     : 2;
    uint8_t present : 1;
} __attribute__((packed)) idt_flags;

typedef struct {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t  zero;
    idt_flags flags;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed)) idt_entry;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr;

idt_entry idt_64[256];
idtr idtr_64;

void (*irq_handlers[16])(irq_regs*, void*);
void* irq_handler_data[16];

void idt_set_gate(size_t gate, void (*function)(void)){
    idt_entry entry = idt_64[gate];

    entry.segment_selector = 0x08;
    idt_flags flags;
    flags.type = 0xE;
    flags.zero = 0;
    flags.dpl = 0;
    flags.present = 1;
    entry.flags = flags;
    entry.reserved = 0;
    entry.zero = 0;

    uint64_t addr = (uint64_t)function;

    entry.offset_low = addr & 0xFFFF;
    entry.offset_middle = (addr >> 16) & 0xFFFF;
    entry.offset_high= addr  >> 32;
}

uint64_t get_cr2(){
    uint64_t value;
    asm volatile("mov %%rax, %%cr2" : :);
    asm volatile("mov %0, %%rax;" : "=m" (value));
    return value;
}

uint64_t get_cr3(){
    uint64_t value;
    asm volatile("mov %%rax, %%cr3; mov %0, %%rax;" : "=m" (value));
    return value;
}

void install_idt(){
    //Set the correct values inside IDTR
    idtr_64.limit = (64 * 16) - 1;

    //Give the IDTR address to the CPU
    asm volatile("lidt %0" : : "m" (idtr_64));
}

void install_isrs(){
    uint16_t gdt = 0x08;

    idt_set_gate(0, isr0);
    idt_set_gate(1, isr1);
    idt_set_gate(2, isr2);
    idt_set_gate(3, isr3);
    idt_set_gate(4, isr4);
    idt_set_gate(5, isr5);
    idt_set_gate(6, isr6);
    idt_set_gate(7, isr7);
    idt_set_gate(8, isr8);
    idt_set_gate(9, isr9);
    idt_set_gate(10, isr10);
    idt_set_gate(11, isr11);
    idt_set_gate(12, isr12);
    idt_set_gate(13, isr13);
    idt_set_gate(14, isr14);
    idt_set_gate(15, isr15);
    idt_set_gate(16, isr16);
    idt_set_gate(17, isr17);
    idt_set_gate(18, isr18);
    idt_set_gate(19, isr19);
    idt_set_gate(20, isr20);
    idt_set_gate(21, isr21);
    idt_set_gate(22, isr22);
    idt_set_gate(23, isr23);
    idt_set_gate(24, isr24);
    idt_set_gate(25, isr25);
    idt_set_gate(26, isr26);
    idt_set_gate(27, isr27);
    idt_set_gate(28, isr28);
    idt_set_gate(29, isr29);
    idt_set_gate(30, isr30);
    idt_set_gate(31, isr31);
}

void remap_irqs(){
    //Restart the both PICs
    port_word_out(0x20, 0x11);
    port_word_out(0xA0, 0x11);

    port_word_out(0x21, 0x20); //Make PIC1 start at 32
    port_word_out(0xA1, 0x28); //Make PIC2 start at 40

    //Setup cascading for both PICs
    port_word_out(0x21, 0x04);
    port_word_out(0xA1, 0x02);

    //8086 mode for both PICs
    port_word_out(0x21, 0x01);
    port_word_out(0xA1, 0x01);

    //Activate all IRQs in both PICs
    port_word_out(0x21, 0x0);
    port_word_out(0xA1, 0x0);
}

void install_irqs(){
    idt_set_gate(32, irq0);
    idt_set_gate(33, irq1);
    idt_set_gate(34, irq2);
    idt_set_gate(35, irq3);
    idt_set_gate(36, irq4);
    idt_set_gate(37, irq5);
    idt_set_gate(38, irq6);
    idt_set_gate(39, irq7);
    idt_set_gate(40, irq8);
    idt_set_gate(41, irq9);
    idt_set_gate(42, irq10);
    idt_set_gate(43, irq11);
    idt_set_gate(44, irq12);
    idt_set_gate(45, irq13);
    idt_set_gate(46, irq14);
    idt_set_gate(47, irq15);
}

void enable_interrupts(){
    asm volatile("sti" : : );
}

char* exceptions_title[32] = {
    "Division by zero",
    "Debugger",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bounds",
    "Invalid Opcode",
    "Coprocessor not available",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid Task State Segment",
    "Segment not present",
    "Stack Fault",
    "General protection fault",
    "Page Fault",
    "Reserved",
    "Math Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void isr_handler(isr_regs regs){
    kprintln("SYSTEM INTERRUPT");
}

void irq_handler(irq_regs* regs){
    //If the IRQ is on the slave controller, send EOI to it
    if(regs->code >= 8){
        port_word_out(0xA0, 0x20);
    }

    //Send EOI to the master controller
    port_word_out(0x20, 0x20);

    //If there is an handler, call it
    if(irq_handlers[regs->code]){
        irq_handlers[regs->code](regs, irq_handler_data[regs->code]);
    }
}

bool register_irq_handler(size_t irq, void (*handler)(irq_regs*, void*), void* data){
    if(irq_handlers[irq]){
        return false;
    }

    if(irq > 15){
        return false;
    }

    irq_handlers[irq] = handler;
    irq_handler_data[irq] = data;

    return true;
}

bool unregister_irq_handler(size_t irq, void (*handler)(irq_regs*, void*)){
    if(!irq_handlers[irq]){
        return false;
    }

    if(irq > 15){
        return false;
    }

    if(irq_handlers[irq] != handler){
        return false;
    }

    irq_handlers[irq] = 0;
    irq_handler_data[irq] = 0;

    return true;
}

void interrupt_init(){
    install_idt();
    install_isrs();
    remap_irqs();
    install_irqs();
    
    enable_interrupts();
}
