#include "../drivers/screen.h"
#include "../cpu/isr.h"
#include "../cpu/gdt.h"
#include "../cpu/page.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include "multiboot.h"

#include <stdint.h>

extern void paget_test(char *msg);

void kernel_main(multiboot_t *mboot_ptr) {
  clear_screen();
  kprint("Hello, I am happy to see you.\n");

  init_gdt();
  isr_install();

  asm volatile("int $1");
  asm volatile("int $2");
  asm volatile("int $3");
  asm volatile("int $4");
  asm volatile("int $13");
  asm volatile("int $14");

  // int a = 3 / 0; // keep interrupting ?

  irq_install();

  init_paging();

  asm volatile("int $14");
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
  }

  kprint("You typed: ");
  kprintln(input);
  kprint("shell$ ");
}
