#include "page.h"
#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "../libc/string.h"
#include "../libc/mem.h"

uint32_t *frames;
uint32_t nframes;

extern uint32_t free_mem_addr;

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

static void set_frame(uint32_t frame_addr) {
  uint32_t frame = frame_addr / 0x1000;
  uint32_t index = INDEX_FROM_BIT(frame);
  uint32_t offset = OFFSET_FROM_BIT(frame);

  frames[index] |= (0x1 << offset);
}

static void clear_frame(uint32_t frame_addr) {
  uint32_t frame = frame_addr / 0x1000;
  uint32_t index = INDEX_FROM_BIT(frame);
  uint32_t offset = OFFSET_FROM_BIT(frame);

  frames[index] &= ~(0x1 << offset);  
}

static uint32_t test_frame(uint32_t frame_addr) {
  uint32_t frame = frame_addr / 0x1000;
  uint32_t index = INDEX_FROM_BIT(frame);
  uint32_t offset = OFFSET_FROM_BIT(frame);

  return (frames[index] & (0x1 << offset));
}

static uint32_t first_frame() {
  uint32_t i, j;
  for (i = 0; i < INDEX_FROM_BIT(nframes); i ++) {
    if (frames[i] != 0xFFFFFFFF) {
      for (j = 0; j < 32; j++) {
        uint32_t toTest = 0x1 << j;
        if (!(frames[i] & toTest)) return i * 4 * 8 + j;
      }
    }
  }

  return (uint32_t)-1;
}

void alloc_frame(page_t *page, int is_kernel, int is_writable) {
  if (page->frame != 0) {
    return;
  } else {
    uint32_t index = first_frame();
    if (index == (uint32_t)-1) {
      kprintln("NO FREE FRAMES");
    }

    set_frame(index * 0x1000);
    page->present = 1;
    page->rw = is_writable ? 1 : 0;
    page->user = is_kernel ? 1 : 0;
    page->frame = index;
  }
}


void free_frame(page_t *page) {
  uint32_t frame;
  if (!(frame = page->frame)) {
    return;
  } else {
    clear_frame(frame);
    page->frame = 0x0;
  }
}

page_directory_t *kernel_directory = 0;
page_directory_t *current_directory = 0;  

void init_paging() {
  uint32_t mem_end_page = 0x1000000; // 16M

  nframes = mem_end_page / 0x1000;
  frames = (uint32_t *)kmalloc_align(INDEX_FROM_BIT(nframes));
  memory_set((uint8_t *)frames, 0, INDEX_FROM_BIT(nframes));

  kernel_directory = (page_directory_t *)kmalloc_align(sizeof(page_directory_t));
  memory_set((uint8_t *)kernel_directory, 0, sizeof(page_directory_t));
  current_directory = kernel_directory;

  uint32_t i = 0;
  while (i < free_mem_addr) {
    alloc_frame(get_page(i, 1, kernel_directory), 0, 0);
    i += 0x1000;
  }

  register_interrupt_handler(14, page_fault);
  register_interrupt_handler(13, page_fault);

  switch_page_directory(kernel_directory);
}

void switch_page_directory(page_directory_t *dir)
{
   current_directory = dir;
   asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysicalAddr));
   uint32_t cr0;
   asm volatile("mov %%cr0, %0": "=r"(cr0));
   cr0 |= 0x80000000; // Enable paging!
   asm volatile("mov %0, %%cr0":: "r"(cr0));
}

page_t *get_page(uint32_t address, int make, page_directory_t *dir) {
  address /= 0x1000;
  uint32_t table_index = address / 1024;
  if (dir->tables[table_index]) {
    return &dir->tables[table_index]->pages[address % 1024];
  } else if (make) {
    uint32_t tmp;
    dir->tables[table_index] = (page_table_t *)kmalloc_phys(sizeof(page_table_t), 1, &tmp);
    memory_set((uint8_t *)dir->tables[table_index], 0, 0x1000);
    dir->tablesPhysicalAddr[table_index] = tmp | 0x7;
    return &dir->tables[table_index]->pages[address % 1024];
  } else {
    return 0;
  }
}

void page_fault(registers_t *regs) {
  uint32_t fault_address;
  asm volatile("mov %%cr2, %0" : "=r" (fault_address));

  int present = !(regs->err_code & 0x1);
  int rw = regs->err_code & 0x2;           // Write operation?
  int us = regs->err_code & 0x4;           // Processor was in user-mode?
  int reserved = regs->err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
  int id = regs->err_code & 0x10;          // Caused by an instruction fetch?

    // Output an error message.
  kprint("Page fault! ( ");
  if (present) kprint("present ");
  if (rw) kprint("read-only ");
  if (us) kprint("user-mode ");
  if (reserved) kprint("reserved ");

  kprint(") at 0x");
  kprint_hex(fault_address);
  kprintln("");
}
