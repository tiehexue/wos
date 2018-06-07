#include "dummyHeap.h"

#include "../drivers/screen.h"
#include "../cpu/page.h"

uint32_t heapStart = 0;
uint32_t heapEnd;
uint32_t currentHeapPosition;

uint32_t placement_address;

extern page_directory_t *kernel_directory;
extern uint32_t paging_enabled;

uint32_t kmalloc_int(uint32_t sz, int align, uint32_t *phys)
{
    if (paging_enabled && currentHeapPosition != heapEnd)
    {
      if (currentHeapPosition == heapEnd) {
        kprintln("OUT OF MEMORY");
      }

      if (align == 1 && (currentHeapPosition & 0x00000FFF) )
        {
            // Align the placement address;
            currentHeapPosition &= 0xFFFFF000;
            currentHeapPosition += 0x1000;
        }
        if (phys)
        {
          page_t *page = get_page((uint32_t)currentHeapPosition, 1, kernel_directory);
          *phys = page->frame*0x1000 + ((uint32_t)currentHeapPosition&0xFFF);
        }

        uint32_t tmp = currentHeapPosition;
        currentHeapPosition += sz;
        return tmp;
    }
    else
    {
        if (align == 1 && (placement_address & 0x00000FFF) )
        {
            // Align the placement address;
            placement_address &= 0xFFFFF000;
            placement_address += 0x1000;
        }
        if (phys)
        {
            *phys = placement_address;
        }
        uint32_t tmp = placement_address;
        placement_address += sz;
        return tmp;
    }
}

void kfree(void *p)
{
  //
}

uint32_t kmalloc_a(uint32_t sz)
{
    return kmalloc_int(sz, 1, 0);
}

uint32_t kmalloc_p(uint32_t sz, uint32_t *phys)
{
    return kmalloc_int(sz, 0, phys);
}

uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys)
{
    return kmalloc_int(sz, 1, phys);
}

uint32_t kmalloc(uint32_t sz)
{
    return kmalloc_int(sz, 0, 0);
}
