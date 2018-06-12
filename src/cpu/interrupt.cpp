#include "interrupt.hpp"
#include "gdt_types.hpp"
#include "ports.hpp"

#include "../drivers/screen.hpp"

#include <algorithms.hpp>

extern "C" {

void isr0();
void isr1();
void isr2();
void isr3();
void isr4();
void isr5();
void isr6();
void isr7();
void isr8();
void isr9();
void isr10();
void isr11();
void isr12();
void isr13();
void isr14();
void isr15();
void isr16();
void isr17();
void isr18();
void isr19();
void isr20();
void isr21();
void isr22();
void isr23();
void isr24();
void isr25();
void isr26();
void isr27();
void isr28();
void isr29();
void isr30();
void isr31();

void irq0();
void irq1();
void irq2();
void irq3();
void irq4();
void irq5();
void irq6();
void irq7();
void irq8();
void irq9();
void irq10();
void irq11();
void irq12();
void irq13();
void irq14();
void irq15();

}

namespace {

struct idt_flags {
    uint8_t type    : 4;
    uint8_t zero    : 1;
    uint8_t dpl     : 2;
    uint8_t present : 1;
} __attribute__((packed));

struct idt_entry {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t  zero;
    idt_flags flags;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct idtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

idt_entry idt_64[64];
idtr idtr_64;

void (*irq_handlers[16])(interrupt::syscall_regs*, void*);
void* irq_handler_data[16];
void (*syscall_handlers[interrupt::SYSCALL_MAX])(interrupt::syscall_regs*);

void idt_set_gate(size_t gate, void (*function)(void), uint16_t gdt_selector, idt_flags flags){
    auto& entry = idt_64[gate];

    entry.segment_selector = gdt_selector;
    entry.flags = flags;
    entry.reserved = 0;
    entry.zero = 0;

    auto function_address = reinterpret_cast<uintptr_t>(function);
    entry.offset_low = function_address & 0xFFFF;
    entry.offset_middle = (function_address >> 16) & 0xFFFF;
    entry.offset_high= function_address  >> 32;
}

void install_idt(){
    //Set the correct values inside IDTR
    idtr_64.limit = (64 * 16) - 1;
    idtr_64.base = reinterpret_cast<size_t>(&idt_64[0]);

    //Clear the IDT
    std::fill_n(reinterpret_cast<size_t*>(idt_64), 64 * sizeof(idt_entry) / sizeof(size_t), 0);

    //Clear the IRQ handlers
    std::fill_n(irq_handlers, 16, nullptr);
    std::fill_n(irq_handler_data, 16, nullptr);

    //Give the IDTR address to the CPU
    asm volatile("lidt [%0]" : : "m" (idtr_64));
}

void install_isrs(){
    idt_set_gate(0, isr0, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(1, isr1, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(2, isr2, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(3, isr3, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(4, isr4, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(5, isr5, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(6, isr6, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(7, isr7, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(8, isr8, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(9, isr9, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(10, isr10, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(11, isr11, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(12, isr12, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(13, isr13, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(14, isr14, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(15, isr15, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(16, isr16, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(17, isr17, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(18, isr18, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(19, isr19, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(20, isr20, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(21, isr21, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(22, isr22, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(23, isr23, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(24, isr24, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(25, isr25, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(26, isr26, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(27, isr27, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(28, isr28, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(29, isr29, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(30, isr30, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(31, isr31, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
}

void remap_irqs(){
    //Restart the both PICs
    out_byte(0x20, 0x11);
    out_byte(0xA0, 0x11);

    out_byte(0x21, 0x20); //Make PIC1 start at 32
    out_byte(0xA1, 0x28); //Make PIC2 start at 40

    //Setup cascading for both PICs
    out_byte(0x21, 0x04);
    out_byte(0xA1, 0x02);

    //8086 mode for both PICs
    out_byte(0x21, 0x01);
    out_byte(0xA1, 0x01);

    //Activate all IRQs in both PICs
    out_byte(0x21, 0x0);
    out_byte(0xA1, 0x0);
}

void install_irqs(){
    idt_set_gate(32, irq0, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(33, irq1, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(34, irq2, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(35, irq3, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(36, irq4, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(37, irq5, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(38, irq6, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(39, irq7, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(40, irq8, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(41, irq9, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(42, irq10, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(43, irq11, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(44, irq12, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(45, irq13, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(46, irq14, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
    idt_set_gate(47, irq15, gdt::LONG_SELECTOR, {gdt::SEG_INTERRUPT_GATE, 0, 0, 1});
}

void enable_interrupts(){
    asm volatile("sti" : : );
}

const char* exceptions_title[32] {
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

} //end of anonymous namespace

extern "C" {

void isr_handler(interrupt::fault_regs regs){
   kprint("INTERRUPTS ISR: ");
   kprint_int(regs.error_no);
   kprint(", ");
   kprint(exceptions_title[regs.error_no]);
   kprint(", ");
   kprint_int(regs.error_code);
   kprintln("");
}

void irq_handler(interrupt::syscall_regs* regs){
    kprint("INTERRUPTS IRQ: ");
    kprint_int(regs->code);
    kprintln("");

    //If the IRQ is on the slave controller, send EOI to it
    if(regs->code >= 8){
        out_byte(0xA0, 0x20);
    }

    //Send EOI to the master controller
    out_byte(0x20, 0x20);

    //If there is an handler, call it
    if(irq_handlers[regs->code]){
        irq_handlers[regs->code](regs, irq_handler_data[regs->code]);
    }
}

} //end of extern "C"

bool interrupt::register_irq_handler(size_t irq, void (*handler)(interrupt::syscall_regs*, void*), void* data){
    if(irq_handlers[irq]){
        //logging::logf(logging::log_level::ERROR, "Register interrupt %u while already registered\n", irq);
        return false;
    }

    if(irq > 15){
        //logging::logf(logging::log_level::ERROR, "Register interrupt %u too high\n", irq);
        return false;
    }

    irq_handlers[irq] = handler;
    irq_handler_data[irq] = data;

    return true;
}

bool interrupt::unregister_irq_handler(size_t irq, void (*handler)(interrupt::syscall_regs*, void*)){
    if(!irq_handlers[irq]){
        //logging::logf(logging::log_level::ERROR, "Unregister interrupt %u while not registered\n", irq);
        return false;
    }

    if(irq > 15){
        //logging::logf(logging::log_level::ERROR, "Unregister interrupt %u too high\n", irq);
        return false;
    }

    if(irq_handlers[irq] != handler){
        //logging::logf(logging::log_level::ERROR, "Unregister wrong irq handler %u\n", irq);
        return false;
    }


    irq_handlers[irq] = nullptr;
    irq_handler_data[irq] = nullptr;

    return true;
}

void interrupt::setup_interrupts(){

    install_idt();
    install_isrs();
    remap_irqs();
    install_irqs();

    enable_interrupts();
}

