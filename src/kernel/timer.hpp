#ifndef TIMER_HPP
#define TIMER_HPP

#include <stdint.h>

namespace timer{

void init(uint32_t freq);

void wait(uint32_t ticks);

}

#endif
