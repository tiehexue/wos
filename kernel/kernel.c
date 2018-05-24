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
  asm volatile("int $6");

  // int a = 3 / 0; // keep interrupting ?

  irq_install();

  paget_test("This should be printed.");
  enablePaging();
  paget_test("This should not be printed, but page fault.");

  kprint("shell$ ");
}

void user_input(char *input) {
  if (strcmp(input, "END") == 0) {
    kprintln("Stopping the CPU, bye!");
    asm volatile("hlt");
  } else if (strcmp(input, "PAGE") == 0) {
    uint32_t phys_addr;
    uint32_t page = kmalloc(1000, 1, &phys_addr);
    char page_str[16] = "";
    int2hex(page, page_str);
    char phys_str[16] = "";
    int2hex(phys_addr, phys_str);
    kprint("Page: ");
    kprint(page_str);
    kprint(", physical address: ");
    kprintln(phys_str);
  }

  kprint("You typed: ");
  kprintln(input);
  kprint("shell$ ");
}
