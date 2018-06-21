#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

void memcpy(uint8_t *dest, uint8_t *src, int nbytes);
void memset(uint8_t *dest, uint8_t val, uint32_t len);

#endif
