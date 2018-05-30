#include "mem.h"
#include "../drivers/screen.h"

void memory_copy(uint8_t *source, uint8_t *dest, int nbytes) {
  int i;
  for (i = 0; i < nbytes; i++) {
    *(dest + i) = *(source + i);  
  }
}

void memory_set(uint8_t *dest, uint8_t val, uint32_t len) {
  uint8_t *temp = (uint8_t *)dest;
  for (; len != 0; len--) *temp++ = val;
}

extern uint32_t end;
uint32_t free_mem_addr = (uint32_t)&end;

uint32_t kmalloc_phys(size_t size, int align, uint32_t *phys_addr) {
  kprintln("Malloc is called now.");
  if (align == 1 && (free_mem_addr & 0x00000FFF)) {
    free_mem_addr &= 0xFFFFF000;
    free_mem_addr += 0x1000;
  }

  if (phys_addr) *phys_addr = free_mem_addr;

  uint32_t ret = free_mem_addr;
  free_mem_addr += size;
  return ret;
}

uint32_t kmalloc_align(size_t size) {
  return kmalloc_phys(size, 1, NULL);
}

uint32_t kmalloc(size_t size) {
  return kmalloc_phys(size, 0, NULL);
}
