#include "page.h"
#include "../drivers/screen.h"
#include "../libc/string.h"

#include <stdint.h>

uint32_t page_directory[ONE_K] __attribute__((aligned(FOUR_K)));
uint32_t first_page_table[ONE_K] __attribute__((aligned(FOUR_K)));

void setPageDirectory() {
  for (int i = 0; i < ONE_K; i++) {
    page_directory[i] = 0x00000002;
  }
}

void setFirstPageTable() {
  for (int i = 0; i < ONE_K; i++) {
    first_page_table[i] = (i * 0x1000) | 3;
  }

  page_directory[0] = ((uint32_t) first_page_table) | 3;
}

void enablePaging() {
  
  setPageDirectory();
  setFirstPageTable();

  uint32_t cr = 0;
  asm volatile("mov %%cr3, %%eax": "=a" (cr));
  kprint("Set page directory, cr3 is: ");
  char str[10];
  int_to_ascii(cr, str);
  kprintln(str);
  asm volatile("mov %%eax, %%cr3" : : "a" (page_directory));

  asm volatile("mov %%cr3, %%eax": "=a" (cr));
  kprint("After setting page directory, cr3 is: ");
  int_to_ascii(cr, str);
  kprintln(str);

  asm volatile("mov %%cr0, %%eax": "=a" (cr));
  kprint("Enable Paging Started, cr0 is: ");
  int_to_ascii(cr, str);
  kprintln(str);
  asm volatile("mov %cr0, %eax");
  asm volatile("or $0x80000001, %eax");
  asm volatile("mov %eax, %cr0");
  kprintln("Enable Paging finished");
}
