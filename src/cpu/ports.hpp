#ifndef PORTS_H
#define PORTS_H

#include <stdint.h>

void out_byte(uint8_t value, uint16_t port);

uint8_t in_byte(uint16_t port);

#endif
