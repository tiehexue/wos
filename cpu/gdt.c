#include "gdt.h"

extern void gdt_flush(uint32_t ptr);
static void gdt_set_entry(uint32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);

gdt_entry_t gdt_entries[5];
gdt_ptr_t gdt_ptr;

void init_gdt() {
  gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
  gdt_ptr.base = (uint32_t) &gdt_entries;

  gdt_set_entry(0, 0, 0, 0, 0);
  gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
  gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
  gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
  gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

  gdt_flush((uint32_t)&gdt_ptr);
}

static void gdt_set_entry(uint32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
  gdt_entries[index].base_low = base & 0xFFFF;
  gdt_entries[index].base_middle = (base >> 16) & 0xFF;
  gdt_entries[index].base_high = (base >> 24) & 0xFF;

  gdt_entries[index].limit_low = limit & 0xFFFF;
  gdt_entries[index].granularity = (limit >> 16) & 0x0F; // limit 16-19

  gdt_entries[index].granularity |= granularity & 0xF0;
  gdt_entries[index].access = access;
}
