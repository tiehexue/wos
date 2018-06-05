#include "timer.h"
#include "../drivers/screen.h"
#include "../cpu/ports.h"
#include "isr.h"
#include "../libc/function.h"
#include "../kernel/task.h"

uint32_t tick = 0;

static void timer_callback(registers_t *regs) {
  tick++;
  switch_task();
  UNUSED(*regs);
}

void init_timer(uint32_t freq) {
  register_interrupt_handler(IRQ0, timer_callback);

  uint32_t divisor = 1193180 / freq;
  uint8_t low = (uint8_t)(divisor & 0xFF);
  uint8_t high = (uint8_t)((divisor >>8) & 0xFF);

  port_byte_out(0x43, 0x36);
  port_byte_out(0x40, low);
  port_byte_out(0x40, high);
}

void wait(uint32_t ticks) {
  kprint("Current tick: ");
  kprint_int(tick);
  kprintln("");

  uint32_t start = tick;
  while (tick - start <= ticks);
}
