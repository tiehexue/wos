#include "../drivers/screen.h"
#include "../cpu/isr.h"
#include "../cpu/gdt.h"
#include "../cpu/timer.h"
#include "../cpu/page.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include "multiboot.h"
#include "heap.h"

#include <stdint.h>

extern void paget_test(char *msg);

uint32_t multiboot_mem_upper = 0;

void kernel_main(multiboot_t *mboot_ptr) {

  multiboot_mem_upper = mboot_ptr->mem_upper * 1024;

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

  asm volatile("int $14");

  uint32_t a = kmalloc(8);
  
  init_paging();

  uint32_t b = kmalloc(8);
  uint32_t c = kmalloc(8);

  kprint("a: ");
  kprint_hex(a);
  kprint(", b: ");
  kprint_hex(b);
  kprint(", c: ");
  kprint_hex(c);
  kprintln("");

  kfree(c);
  kfree(b);

  uint32_t d = kmalloc(12);
  kprint("d: ");
  kprint_hex(d);
  kprintln("");

  for(;;) {
    wait(40);
    uint32_t phys_addr;
    uint32_t page = kmalloc_phys(0x500000, &phys_addr);
    char page_str[16] = "";
    int2hex(page, page_str);
    char phys_str[16] = "";
    int2hex(phys_addr, phys_str);
    kprint("Page: ");
    kprint(page_str);
    kprint(", physical address: ");
    kprintln(phys_str);
  }

  kprint("shell$ ");
  for(;;);
}

void user_input(char *input) {
  if (strcmp(input, "END") == 0) {
    kprintln("Stopping the CPU, bye!");
    asm volatile("hlt");
  } else if (strcmp(input, "PAGE") == 0) {
    uint32_t phys_addr;
    uint32_t page = kmalloc_phys(0x1000, &phys_addr);
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
