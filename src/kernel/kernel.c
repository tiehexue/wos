#include "../cpu/interrupt.h"
#include "../drivers/screen.h"

void kernel_main() {
  clear_screen();

  interrupt_init();

  //asm volatile("int $1");

  kprint("shell$ ");
  for(;;);
}
