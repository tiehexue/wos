#include "memory.hpp"

void memcpy(uint8_t *dest, uint8_t *src, int nbytes) {
  for (int i = 0; i < nbytes; i++) {
    *(dest + i) = *(src + i);  
  }
}

void memset(uint8_t *dest, uint8_t val, uint32_t len) {
  while(len--) *dest++ = val;
}
