#include "../cpu/interrupt.h"
#include "../drivers/screen.h"

void kernel_main() {
  clear_screen();

  interrupt_init();

  kprint("shell$ ");
  for(;;);
}
