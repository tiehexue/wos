#include "page.h"
#include "../kernel/heap.h"
#include "../libc/memory.h"
#include "../drivers/screen.h"

uint32_t paging_enabled = 0;
page_directory_t *kernel_directory=0;

static uint32_t *frames_index;
static uint32_t frames_count;
static uint32_t current_frame_index = 0;

extern uint32_t placement_address;
extern uint32_t multiboot_mem_upper;

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

static void set_frame(uint32_t frame_addr) {
  uint32_t frame = frame_addr/0x1000;
  uint32_t index = INDEX_FROM_BIT(frame);
  uint32_t off = OFFSET_FROM_BIT(frame);
  frames_index[index] |= (0x1 << off);
}

static void clear_frame(uint32_t frame_addr) {
  uint32_t frame = frame_addr/0x1000;
  uint32_t index = INDEX_FROM_BIT(frame);
  uint32_t off = OFFSET_FROM_BIT(frame);
  frames_index[index] &= ~(0x1 << off);
}

static uint32_t test_frame(uint32_t frame_addr) {
  uint32_t frame = frame_addr/0x1000;
  uint32_t index = INDEX_FROM_BIT(frame);
  uint32_t off = OFFSET_FROM_BIT(frame);
  return (frames_index[index] & (0x1 << off));
}

static uint32_t first_frame() {
  return current_frame_index++;
}

void alloc_frame(page_t *page, int is_kernel, int is_writeable) {
  if (page->frame != 0) {
    return;
  } else {
    uint32_t index = first_frame();

    if (index == frames_count - 1) {
      kprint("HIT TO END MEMORY WITH INDEX: ");
      kprintln_int(frames_count);
    }
  
    set_frame(index*0x1000);
    page->present = 1;
    page->rw = (is_writeable==1)?1:0;
    page->user = (is_kernel==1)?0:1;
    page->frame = index;
  }
}

void free_frame(page_t *page) {
  uint32_t frame;
  if (!(frame=page->frame)) {
    return;
  } else {
    clear_frame(frame);
    page->frame = 0x0;
  }
}

void initialise_paging() {
  frames_count = multiboot_mem_upper / 0x1000;
  frames_index = (uint32_t*)kmalloc(INDEX_FROM_BIT(frames_count) * sizeof(uint32_t));
  memset(frames_index, 0, INDEX_FROM_BIT(frames_count) * sizeof(uint32_t));
    

  uint32_t phys;
  kernel_directory = (page_directory_t *)kmalloc_align(sizeof(page_directory_t));
  memset(kernel_directory, 0, sizeof(page_directory_t));
  kernel_directory->physical = (uint32_t)kernel_directory->physicalTables;

  uint32_t i = 0;
  while (i < (multiboot_mem_upper & 0xFFFFF000)) {
    alloc_frame(get_page(i, 1, kernel_directory), 0, 0);
    i += 0x1000;
  }

  register_interrupt_handler(14, page_fault);

  switch_page_directory(kernel_directory);

  paging_enabled = 1;
}

void switch_page_directory(page_directory_t *dir) {
  asm volatile("mov %0, %%cr3":: "r"(dir->physical));
  uint32_t cr0;
  asm volatile("mov %%cr0, %0": "=r"(cr0));
  cr0 |= 0x80000000; // Enable paging!
  asm volatile("mov %0, %%cr0":: "r"(cr0));
}

page_t *get_page(uint32_t address, int make, page_directory_t *dir) {
    
  address /= 0x1000;
  
  uint32_t table_index = address / 1024;

  if (dir->tables[table_index]) {
    return &dir->tables[table_index]->pages[address%1024];
  } else if(make) {
    uint32_t tmp;
    dir->tables[table_index] = (page_table_t*)kmalloc_align_physical(sizeof(page_table_t), &tmp);
    memset(dir->tables[table_index], 0, sizeof(page_table_t));
    dir->physicalTables[table_index] = tmp | 0x7; // PRESENT, RW, US.
    return &dir->tables[table_index]->pages[address%1024];
  } else {
    return 0;
  }
}

void page_fault(registers_t regs) {
  uint32_t faulting_address;
  asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

  page_t *page = get_page(faulting_address, 0, kernel_directory);
  uint32_t dirIndex = faulting_address >> 22;
  uint32_t tableIndex = (faulting_address << 10) >> 22;

  kprint("DIRECTORY PAGE FRAME: ");
  kprint_int(dirIndex);
  kprint(" ");
  kprint_int(tableIndex);
  kprint(" ");
  kprintln_int(page->frame);
    
  int present = !(regs.err_code & 0x1);
  int rw = regs.err_code & 0x2;
  int us = regs.err_code & 0x4;
  int reserved = regs.err_code & 0x8;
  int id = regs.err_code & 0x10;

    
  kprint("Page fault! ( ");
  if (present) {kprint("present ");}
  if (rw) {kprint("read-only ");}
  if (us) {kprint("user-mode ");}
  if (reserved) {kprint("reserved ");}
  kprint(") at ");
  kprint_hex(faulting_address);
  kprint(" - EIP: ");
  kprintln_hex(regs.eip);
}
