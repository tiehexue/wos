#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef uint64_t size_t;

typedef double sse_128 __attribute__((vector_size(16)));

#endif
