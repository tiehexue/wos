#include "../cpu/interrupt.h"
#include "../drivers/screen.h"

void kernel_main() {
  clear_screen();

  interrupt_init();

//  asm volatile("int $1");
//  asm volatile("int $2");
//  asm volatile("int $3");
//  asm volatile("int $4");
 // asm volatile("int $13");
//  asm volatile("int $14");

  kprint("shell$ ");
  for(;;);
}
