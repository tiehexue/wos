#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

#include "isr.h"

typedef struct page {
  uint32_t present   : 1;
  uint32_t rw        : 1;
  uint32_t user      : 1;
  uint32_t accessed  : 1;
  uint32_t dirty     : 1;
  uint32_t unused    : 7;
  uint32_t frame     : 20;
} page_t;

typedef struct page_table
{
  page_t pages[1024];  
} page_table_t;

typedef struct page_directory
{
  page_table_t *tables[1024];
  uint32_t tablesPhysicalAddr[1024];
  uint32_t directoryPhysicalAddr;
} page_directory_t;

void init_paging();

page_t * get_page(uint32_t address, int make, page_directory_t *dir);

void switch_page_directory(page_directory_t *new);

void page_fault(registers_t *regs);

void alloc_frame(page_t *page, int is_kernel, int is_writable);

void free_frame(page_t *page);

#endif