#include "ports.hpp"

void out_byte(uint8_t value, uint16_t port){
    asm volatile("out %1, %0" : : "a" (value), "dN" (port));
}

uint8_t in_byte(uint16_t port){
    uint8_t value;
    asm volatile("in %0,%1" : "=a" (value) : "dN" (port));
    return value;
}
