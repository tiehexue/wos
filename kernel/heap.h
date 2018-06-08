#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>

uint32_t kmalloc_align(uint32_t sz);

uint32_t kmalloc_physical(uint32_t sz, uint32_t *phys);

uint32_t kmalloc_align_physical(uint32_t sz, uint32_t *phys);

uint32_t kmalloc(uint32_t sz);

void kfree(void *p);

#endif
