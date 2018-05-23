#include "../drivers/screen.h"
#include "../cpu/isr.h"
#include "../libc/string.h"
#include "../libc/mem.h"

void main() {
  clear_screen();
  kprint("Hello, I am happy to see you.\n");

  isr_install();

  __asm__ __volatile__("int $1");
  __asm__ __volatile__("int $2");
  __asm__ __volatile__("int $3");
  __asm__ __volatile__("int $4");
  __asm__ __volatile__("int $5");
  __asm__ __volatile__("int $6");

  __asm__ __volatile__("int $12");
  // int a = 3 / 0; // keep interrupting ?
  __asm__ __volatile__("int $3");

  irq_install();
  kprint("shell$ ");
}

void user_input(char *input) {
  if (strcmp(input, "END") == 0) {
    kprintln("Stopping the CPU, bye!");
    asm volatile("hlt");
  } else if (strcmp(input, "PAGE") == 0) {
    u32 phys_addr;
    u32 page = kmalloc(1000, 1, &phys_addr);
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
