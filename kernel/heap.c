#include "heap.h"

#include "../drivers/screen.h"
#include "../cpu/page.h"

uint32_t placement_address;

extern uint32_t multiboot_mem_upper;

static uint32_t kmalloc_internal(uint32_t sz, int align, uint32_t *phys) {

  if (align == 1 && (placement_address & 0x00000FFF)) {
    placement_address &= 0xFFFFF000;
    placement_address += 0x1000;
  }

  if (placement_address + sz > multiboot_mem_upper) {
    panic("OUT OF MEMORY");
  }

  if (phys) {
    *phys = placement_address;
  }

  uint32_t tmp = placement_address;
  placement_address += sz;
  return tmp;
}

void kfree(void *p) {
  //
}

uint32_t kmalloc_align(uint32_t sz) {
  return kmalloc_internal(sz, 1, 0);
}

uint32_t kmalloc_physical(uint32_t sz, uint32_t *phys) {
  return kmalloc_internal(sz, 0, phys);
}

uint32_t kmalloc_align_physical(uint32_t sz, uint32_t *phys) {
  return kmalloc_internal(sz, 1, phys);
}

uint32_t kmalloc(uint32_t sz) {
  return kmalloc_internal(sz, 0, 0);
}
