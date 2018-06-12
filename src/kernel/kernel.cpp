#include "../cpu/interrupt.hpp"
#include "../drivers/screen.hpp"

extern "C" {
void kernel_main() {
  clear_screen();

  //interrupt::setup_interrupts();

  //asm volatile("int $1");

  kprint("shell$ ");
  for(;;);
}
}
