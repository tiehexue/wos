#include "page.h"
#include "../drivers/screen.h"
#include "../libc/string.h"
#include "../libc/mem.h"

#include <stdint.h>

uint32_t *page_directory;
uint32_t *page_tables;

void setPageDirectory() {
  page_directory = kmalloc(sizeof(uint32_t) * ONE_K);
  page_tables = kmalloc(sizeof(uint32_t) * ONE_K * ONE_K);

  for (int i = 0; i < ONE_K; i++) {
    page_directory[i] = 0x00000002;
  }
}

void setPageTable(int index) {
  for (int i = 0; i < ONE_K; i++) {
    page_tables[index * ONE_K + i] = (index * FOUR_M + (i * FOUR_K)) | 3;
  }

  page_directory[index] = ((uint32_t) (page_tables + index * FOUR_K)) | 3;
}

void enablePaging() {
  
  setPageDirectory();
  
  for (int i = 0; i < ONE_K; i ++) setPageTable(i);

  asm volatile("mov %%eax, %%cr3" : : "a" (page_directory));
  asm volatile("mov %cr0, %eax");
  asm volatile("or $0x80000001, %eax");
  asm volatile("mov %eax, %cr0");

  kprintln("Enable Paging finished");
}
