//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "gdt.hpp"
#include "ports.hpp"

void disable_interrupts(){
    asm volatile ("cli");
}

void gdt::enable_a20_gate(){

    //TODO This should really be improved:
    // 1. Test if a20 already enabled
    // 2- Use several methods of enabling if necessary until one succeeds

    //Enable A20 gate using fast method
    auto port_a = in_byte(0x92);
    port_a |=  0x02;
    port_a &= ~0x01;
    out_byte(port_a, 0x92);
}

void setup_idt(){
    static const gdt::gdt_ptr null_idt = {0, 0};
    asm volatile("lidt %0" : : "m" (null_idt));
}

gdt::gdt_descriptor_t null_descriptor(){
    gdt::gdt_descriptor_t descriptor;

    //zero-out the descriptor
    *(reinterpret_cast<uint64_t*>(&descriptor)) = 0;

    return descriptor;
}

gdt::gdt_descriptor_t code_32_descriptor(){
    gdt::gdt_descriptor_t descriptor;

    descriptor.type = gdt::SEG_CODE_EXRD;

    descriptor.base_low = 0;
    descriptor.base_high = 0;
    descriptor.limit_low = 0xFFFF;
    descriptor.limit_high = 0xF;
    descriptor.always_1 = 1;
    descriptor.dpl = 0;
    descriptor.present = 1;
    descriptor.avl = 0;
    descriptor.big = 1;
    descriptor.long_mode = 0;
    descriptor.granularity = 1;

    return descriptor;
}

gdt::gdt_descriptor_t code_64_descriptor(){
    gdt::gdt_descriptor_t descriptor;

    descriptor.type = gdt::SEG_CODE_EXRD;

    descriptor.base_low = 0;
    descriptor.base_high = 0;
    descriptor.limit_low = 0xFFFF;
    descriptor.limit_high = 0xF;
    descriptor.always_1 = 1;
    descriptor.dpl = 0;
    descriptor.present = 1;
    descriptor.avl = 0;
    descriptor.big = 0;
    descriptor.long_mode = 1;
    descriptor.granularity = 1;

    return descriptor;
}

gdt::gdt_descriptor_t user_code_64_descriptor(){
    gdt::gdt_descriptor_t descriptor;

    descriptor.type = gdt::SEG_CODE_EXRD;

    descriptor.base_low = 0;
    descriptor.base_high = 0;
    descriptor.limit_low = 0xFFFF;
    descriptor.limit_high = 0xF;
    descriptor.always_1 = 1;
    descriptor.dpl = 3;
    descriptor.present = 1;
    descriptor.avl = 0;
    descriptor.big = 0;
    descriptor.long_mode = 1;
    descriptor.granularity = 1;

    return descriptor;
}

gdt::gdt_descriptor_t data_descriptor(){
    gdt::gdt_descriptor_t descriptor;

    descriptor.type = gdt::SEG_DATA_RDWR;

    descriptor.base_low = 0;
    descriptor.base_high = 0;
    descriptor.limit_low = 0xFFFF;
    descriptor.limit_high = 0xF;
    descriptor.always_1 = 1;
    descriptor.dpl = 0;
    descriptor.present = 1;
    descriptor.avl = 0;
    descriptor.big = 1;
    descriptor.long_mode = 0;
    descriptor.granularity = 1;

    return descriptor;
}

gdt::gdt_descriptor_t user_data_descriptor(){
    gdt::gdt_descriptor_t descriptor;

    descriptor.type = gdt::SEG_DATA_RDWR;

    descriptor.base_low = 0;
    descriptor.base_high = 0;
    descriptor.limit_low = 0xFFFF;
    descriptor.limit_high = 0xF;
    descriptor.always_1 = 1;
    descriptor.dpl = 3;
    descriptor.present = 1;
    descriptor.avl = 0;
    descriptor.big = 0;
    descriptor.long_mode = 1;
    descriptor.granularity = 1;

    return descriptor;
}

//TODO On some machines, this should be aligned to 16 bits
gdt::gdt_descriptor_t gdt_entries[8];

gdt::gdt_ptr gdtr;

void gdt::setup_gdt(){
    //1. Init GDT descriptor
    gdt_entries[0] = null_descriptor();
    gdt_entries[1] = code_32_descriptor();
    gdt_entries[2] = data_descriptor();
    gdt_entries[3] = code_64_descriptor();
    gdt_entries[4] = user_code_64_descriptor();
    gdt_entries[5] = user_data_descriptor();

    //2. Init TSS Descriptor

    uint32_t base = 0x904A0;
    uint32_t limit = base + sizeof(gdt::task_state_segment_t);

    auto tss_selector = reinterpret_cast<gdt::tss_descriptor_t*>(&gdt_entries[6]);
    tss_selector->type = gdt::SEG_TSS_AVAILABLE;
    tss_selector->always_0_1 = 0;
    tss_selector->always_0_2 = 0;
    tss_selector->always_0_3 = 0;
    tss_selector->dpl = 3;
    tss_selector->present = 1;
    tss_selector->avl = 0;
    tss_selector->granularity = 0;

    tss_selector->base_low = base & 0xFFFFFF;              //Bottom 24 bits
    tss_selector->base_middle = (base & 0xFF000000) >> 24; //Top 8 bits
    tss_selector->base_high = 0;                           //Top 32 bits are clear

    tss_selector->limit_low = limit & 0xFFFF;              //Low 16 bits
    tss_selector->limit_high = (limit & 0xF0000) >> 16;      //Top 4 bits

    //3. Init the GDT Pointer

    gdtr.length  = sizeof(gdt_entries) - 1;
    gdtr.pointer = reinterpret_cast<uint32_t>(&gdt_entries);

    //4. Load the GDT

    asm volatile("lgdt [%0]" : : "m" (gdtr));
}

void gdt::flush_tss(){
    asm volatile("mov ax, %0; ltr ax;" : : "i" (gdt::TSS_SELECTOR + 0x3) : "rax");
}

