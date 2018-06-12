#include "timer.hpp"
#include "../drivers/screen.hpp"
#include "../cpu/ports.hpp"
#include "../cpu/interrupt.hpp"

uint32_t tick = 0;

static void timer_callback(interrupt::syscall_regs *regs, void *data) {
  kprintln("TICK");
  tick++;
}

void timer::init(uint32_t freq) {
  interrupt::register_irq_handler(0, timer_callback, 0);

  uint32_t divisor = 1193180 / freq;
  uint8_t low = (uint8_t)(divisor & 0xFF);
  uint8_t high = (uint8_t)((divisor >>8) & 0xFF);

  out_byte(0x36, 0x43);
  out_byte(low, 0x40);
  out_byte(high, 0x40);
}

void timer::wait(uint32_t ticks) {
  kprint("Current tick: ");
  kprint_int(tick);
  kprintln("");

  uint32_t start = tick;
  while (tick - start <= ticks);
}
