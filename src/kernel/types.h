#ifndef TYPES_H
#define TYPES_H

typedef uint8_t bool;

#define true 1
#define false 0

typedef uint64_t size_t;

typedef double sse_128 __attribute__((vector_size(16)));

#endif
