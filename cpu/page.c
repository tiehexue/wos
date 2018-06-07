// paging.c -- Defines the interface for and structures relating to paging.
//             Written for JamesM's kernel development tutorials.

#include "page.h"
//#include "../kernel/heap.h"
#include "../kernel/dummyHeap.h"
#include "../libc/mem.h"
#include "../drivers/screen.h"

// The kernel's page directory
page_directory_t *kernel_directory=0;

// The current page directory;
page_directory_t *current_directory=0;

// A bitset of frames - used or free.
uint32_t *frames;
uint32_t nframes;
uint32_t current_frame_index = 0;
uint32_t paging_enabled = 0;

// Defined in kheap.c
extern uint32_t placement_address;
//extern heap_t *kheap;
extern uint32_t multiboot_mem_upper;

extern uint32_t heapStart;
extern uint32_t heapEnd;
extern uint32_t currentHeapPosition;

extern void copy_page_physical(uint32_t, uint32_t);

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

// Static function to set a bit in the frames bitset
static void set_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr/0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

// Static function to clear a bit in the frames bitset
static void clear_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr/0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
}

// Static function to test if a bit is set.
static uint32_t test_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr/0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}

// Static function to find the first free frame.
static uint32_t first_frame()
{
    return current_frame_index++;
//    uint32_t i, j;
//    for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
//    {
//        if (frames[i] != 0xFFFFFFFF) // nothing free, exit early.
//        {
//            // at least one bit is free here.
//            for (j = 0; j < 32; j++)
//            {
//                uint32_t toTest = 0x1 << j;
//                if ( !(frames[i]&toTest) )
//                {
//                    return i*4*8+j;
//                }
//            }
//        }
//    }
//
//    return 0;
}

// Function to allocate a frame.
void alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
    if (page->frame != 0)
    {
        return;
    }
    else
    {
        uint32_t idx = first_frame();

       // if (idx >= 0) {
          kprint("FRAME INDEX: ");
          kprint_int(idx);
          kprintln("");
       // }

        if (idx == (nframes - 1))
        {
            PANIC("OUT OF MEMORY");
        }
        set_frame(idx*0x1000);
        page->present = 1;
        page->rw = (is_writeable==1)?1:0;
        page->user = (is_kernel==1)?0:1;
        page->frame = idx;
    }
}

// Function to deallocate a frame.
void free_frame(page_t *page)
{
    uint32_t frame;
    if (!(frame=page->frame))
    {
        return;
    }
    else
    {
        clear_frame(frame);
        page->frame = 0x0;
    }
}

void initialise_paging()
{
    // The size of physical memory. For the moment we 
    // assume it is 16MB big.
    uint32_t mem_end_page = multiboot_mem_upper;
    
    nframes = mem_end_page / 0x1000;
    frames = (uint32_t*)kmalloc(INDEX_FROM_BIT(nframes) * sizeof(uint32_t));
    memory_set(frames, 0, INDEX_FROM_BIT(nframes) * sizeof(uint32_t));
    
    // Let's make a page directory.
    uint32_t phys;
    kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
    memory_set(kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physicalAddr = (uint32_t)kernel_directory->tablesPhysical;

    heapStart = (placement_address & 0xFFFFF000) + 0x5000;
    heapEnd = mem_end_page & 0xFFFFF000;

    uint32_t i = 0;
    for (i = heapStart; i < heapEnd; i += 0x1000)
        get_page(i, 1, kernel_directory);

    while (i < heapStart)
    {
        // Kernel code is readable but not writeable from userspace.
        alloc_frame(get_page(i, 1, kernel_directory), 0, 1);
        i += 0x1000;
    }

    for (i = heapStart; i < heapEnd; i += 0x1000)
        alloc_frame(get_page(i, 1, kernel_directory), 0, 1);

    register_interrupt_handler(14, page_fault);

    switch_page_directory(kernel_directory);

    paging_enabled = 1;

    currentHeapPosition = heapStart;

    current_directory = clone_directory(kernel_directory);
    switch_page_directory(current_directory);
}

void switch_page_directory(page_directory_t *dir)
{
    current_directory = dir;
    asm volatile("mov %0, %%cr3":: "r"(dir->physicalAddr));
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

page_t *get_page(uint32_t address, int make, page_directory_t *dir)
{
    // Turn the address into an index.
    address /= 0x1000;
    // Find the page table containing this address.
    uint32_t table_idx = address / 1024;

    if (dir->tables[table_idx]) // If this table is already assigned
    {
        return &dir->tables[table_idx]->pages[address%1024];
    }
    else if(make)
    {
        uint32_t tmp;
        dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
        memory_set(dir->tables[table_idx], 0, 0x1000);
        dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
        return &dir->tables[table_idx]->pages[address%1024];
    }
    else
    {
        return 0;
    }
}


void page_fault(registers_t regs)
{
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    page_t *page = get_page(faulting_address, 0, kernel_directory);
    uint32_t dirIndex = faulting_address >> 22;
    uint32_t tableIndex = (faulting_address << 10) >> 22;
    kprint("DIRECTORY PAGE FRAME: ");
    kprint_int(dirIndex);kprint(" ");
    kprint_int(tableIndex);kprint(" ");
    kprint_int(page->frame);kprintln("");
    
    // The error code gives us details of what happened.
    int present   = !(regs.err_code & 0x1); // Page not present
    int rw = regs.err_code & 0x2;           // Write operation?
    int us = regs.err_code & 0x4;           // Processor was in user-mode?
    int reserved = regs.err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
    int id = regs.err_code & 0x10;          // Caused by an instruction fetch?

    // Output an error message.
    kprint("Page fault! ( ");
    if (present) {kprint("present ");}
    if (rw) {kprint("read-only ");}
    if (us) {kprint("user-mode ");}
    if (reserved) {kprint("reserved ");}
    kprint(") at ");
    kprint_hex(faulting_address);
    kprint(" - EIP: ");
    kprint_hex(regs.eip);
    kprint("\n");
    PANIC("Page fault");
}

static page_table_t *clone_table(page_table_t *src, uint32_t *physAddr)
{
    // Make a new page table, which is page aligned.
    page_table_t *table = (page_table_t*)kmalloc_ap(sizeof(page_table_t), physAddr);
    // Ensure that the new table is blank.
    memory_set(table, 0, sizeof(page_directory_t));

    // For every entry in the table...
    int i;
    for (i = 0; i < 1024; i++)
    {
        // If the source entry has a frame associated with it...
        if (src->pages[i].frame)
        {
            // Get a new frame.
            alloc_frame(&table->pages[i], 0, 0);
            // Clone the flags from source to destination.
            if (src->pages[i].present) table->pages[i].present = 1;
            if (src->pages[i].rw) table->pages[i].rw = 1;
            if (src->pages[i].user) table->pages[i].user = 1;
            if (src->pages[i].accessed) table->pages[i].accessed = 1;
            if (src->pages[i].dirty) table->pages[i].dirty = 1;
            // Physically copy the data across. This function is in process.s.
            copy_page_physical(src->pages[i].frame*0x1000, table->pages[i].frame*0x1000);
        }
    }
    return table;
}

page_directory_t *clone_directory(page_directory_t *src)
{
    uint32_t phys;
    // Make a new page directory and obtain its physical address.
    page_directory_t *dir = (page_directory_t*)kmalloc_ap(sizeof(page_directory_t), &phys);
    // Ensure that it is blank.
    memory_set(dir, 0, sizeof(page_directory_t));

    // Get the offset of tablesPhysical from the start of the page_directory_t structure.
    uint32_t offset = (uint32_t)dir->tablesPhysical - (uint32_t)dir;

    // Then the physical address of dir->tablesPhysical is:
    dir->physicalAddr = phys + offset;

    // Go through each page table. If the page table is in the kernel directory, do not make a new copy.
    int i;
    for (i = 0; i < 1024; i++)
    {
        if (!src->tables[i])
            continue;

        if (kernel_directory->tables[i] == src->tables[i])
        {
            // It's in the kernel, so just use the same pointer.
            dir->tables[i] = src->tables[i];
            dir->tablesPhysical[i] = src->tablesPhysical[i];
        }
        else
        {
            // Copy the table.
            uint32_t phys;
            dir->tables[i] = clone_table(src->tables[i], &phys);
            dir->tablesPhysical[i] = phys | 0x07;
        }
    }
    return dir;
}
