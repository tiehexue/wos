#include "../cpu/gdt.hpp"
#include "../cpu/interrupt.hpp"
#include "../drivers/screen.hpp"
#include "../drivers/keyboard.hpp"
#include "timer.hpp"

extern "C" {
void kernel_main() {

  clear_screen();

  // gdt::enable_a20_gate();
  
  // gdt::setup_gdt();
  // gdt::flush_tss();

  interrupt::setup_interrupts();

  timer::init(50);
  init_keyboard();

  kprint("shell$ ");

  //asm volatile("int 0x2");

  for(;;);
}
}
