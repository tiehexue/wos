#include "page.h"
#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include "../kernel/heap.h"

uint32_t *frames;
uint32_t nframes;

extern uint32_t placement_address;
extern heap_t *global_heap;

page_directory_t *kernel_directory = 0;
page_directory_t *current_directory = 0;  

extern uint32_t multiboot_mem_upper;
extern void copy_page_physical(uint32_t, uint32_t);

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
  for (i = 0; i < (nframes / 8); i ++) {
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

void init_paging() {
  uint32_t mem_end_page = multiboot_mem_upper;

  nframes = mem_end_page / 0x1000;
  frames = (uint32_t *)kmalloc(nframes / 8);
  memory_set((uint8_t *)frames, 0, nframes / 8);

  kernel_directory = (page_directory_t *)kmalloc_align(sizeof(page_directory_t));
  memory_set((uint8_t *)kernel_directory, 0, sizeof(page_directory_t));
  kernel_directory->directoryPhysicalAddr = (uint32_t)kernel_directory->tablesPhysicalAddr;

  uint32_t i = 0;

  for (i = HEAP_START; i < HEAP_START + HEAP_INIT_SIZE; i += 0x1000)
    get_page(i, 1, kernel_directory);

  i = 0;
  while (i < (placement_address + 0x1000)) {
    alloc_frame(get_page(i, 1, kernel_directory), 0, 0);
    i += 0x1000;
  }

  for (i = HEAP_START; i < HEAP_START + HEAP_INIT_SIZE; i += 0x1000)
    alloc_frame(get_page(i, 1, kernel_directory), 0, 1);

  register_interrupt_handler(14, page_fault);
  //register_interrupt_handler(13, page_fault);

  switch_page_directory(kernel_directory);

  global_heap = create_heap(HEAP_START, HEAP_START + HEAP_INIT_SIZE, 0x100000, 0, 0);

  current_directory = clone_directory(kernel_directory);
  switch_page_directory(current_directory);
}

void switch_page_directory(page_directory_t *dir)
{
   current_directory = dir;
   asm volatile("mov %0, %%cr3":: "r"(dir->directoryPhysicalAddr));
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
    dir->tables[table_index] = (page_table_t *)kmalloc_align_phys(sizeof(page_table_t), &tmp);
    memory_set((uint8_t *)dir->tables[table_index], 0, sizeof(page_table_t));
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

  kprint(") at ");
  kprint_hex(fault_address);
  kprintln("");
}

static page_table_t *clone_table(page_table_t *src, uint32_t *physAddr) {
  page_table_t *table = (page_table_t*)kmalloc_align_phys(sizeof(page_table_t), physAddr);
  memory_set((uint8_t *)table, 0, sizeof(page_directory_t));

  int i;
  for (i = 0; i < 1024; i++) {
    if (src->pages[i].frame) {
      
      alloc_frame(&table->pages[i], 0, 0);
      if (src->pages[i].present) table->pages[i].present = 1;
      if (src->pages[i].rw) table->pages[i].rw = 1;
      if (src->pages[i].user) table->pages[i].user = 1;
      if (src->pages[i].accessed) table->pages[i].accessed = 1;
      if (src->pages[i].dirty) table->pages[i].dirty = 1;
      // Physically copy the data across. This function is in process.s.
      copy_page_physical(src->pages[i].frame * 0x1000, table->pages[i].frame * 0x1000);
    }
  }
  return table;
}

page_directory_t *clone_directory(page_directory_t *src) {
  uint32_t phys;
  // Make a new page directory and obtain its physical address.
  page_directory_t *dir = (page_directory_t*)kmalloc_align_phys(sizeof(page_directory_t), &phys);
  // Ensure that it is blank.
  memory_set((uint8_t *)dir, 0, sizeof(page_directory_t));

  // Get the offset of tablesPhysicalAddr from the start of the page_directory_t structure.
  uint32_t offset = (uint32_t)dir->tablesPhysicalAddr - (uint32_t)dir;

  // Then the physical address of dir->tablesPhysicalAddr is:
  dir->directoryPhysicalAddr = phys + offset;

  // Go through each page table. If the page table is in the kernel directory, do not make a new copy.
  int i;
  for (i = 0; i < 1024; i++) {
    if (!src->tables[i]) continue;

    if (kernel_directory->tables[i] == src->tables[i]) {
            // It's in the kernel, so just use the same pointer.
      dir->tables[i] = src->tables[i];
      dir->tablesPhysicalAddr[i] = src->tablesPhysicalAddr[i];
    } else {
      // Copy the table.
      uint32_t phys;
      dir->tables[i] = clone_table(src->tables[i], &phys);
      dir->tablesPhysicalAddr[i] = phys | 0x07;
    }
  }
  return dir;
}
