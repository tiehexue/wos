#include "../drivers/screen.h"
#include "../cpu/isr.h"
#include "../cpu/page.h"
#include "../libc/string.h"
#include "../libc/mem.h"

#include <stdint.h>

extern void paget_test(char *msg);

void kernel_main() {
  clear_screen();
  kprint("Hello, I am happy to see you.\n");

  isr_install();

  asm volatile("int $1");
  asm volatile("int $2");
  asm volatile("int $3");
  asm volatile("int $4");
  asm volatile("int $5");
  asm volatile("int $13");

  // int a = 3 / 0; // keep interrupting ?

  irq_install();

  kprint("shell$ ");

  for(;;);
}

void user_input(char *input) {
  if (strcmp(input, "END") == 0) {
    kprintln("Stopping the CPU, bye!");
    asm volatile("hlt");
  } else if (strcmp(input, "PAGE") == 0) {
    uint32_t phys_addr;
    uint32_t page = kmalloc_phys(0x1000, 1, &phys_addr);
    char page_str[16] = "";
    int2hex(page, page_str);
    char phys_str[16] = "";
    int2hex(phys_addr, phys_str);
    kprint("Page: ");
    kprint(page_str);
    kprint(", physical address: ");
    kprintln(phys_str);
  } else if (strcmp(input, "JMP") == 0) {
    asm volatile("jmp 0x910000");
  } else if (strcmp(input, "PAGING") == 0) {
    enablePaging();
  }

  kprint("You typed: ");
  kprintln(input);
  kprint("shell$ ");
}
