#include "heap.h"
#include "../cpu/page.h"
#include "../drivers/screen.h"

extern uint32_t end;

uint32_t placement_address = (uint32_t) &end;

extern page_directory_t *kernel_directory;

heap_t *global_heap = 0;

static uint32_t _kmalloc(uint32_t size, uint8_t align, uint32_t *phys) {
  uint32_t returnAdrr = 0;

  if (global_heap != 0) {
    void *addr = alloc(size, align, global_heap);
    if (phys) {
      page_t *page = get_page((uint32_t)addr, 0, kernel_directory);
      *phys = page->frame * 0x1000 + (uint32_t)((uint32_t)addr & 0xFFF);
    }

    returnAdrr = (uint32_t)addr;
  } else {
    if (align == 1 && (placement_address & 0x00000FFF)) {
      placement_address &= 0xFFFFF000;
      placement_address += 0x1000;
    }

    if (phys) {
      *phys = placement_address;
    }

    returnAdrr = placement_address;
    placement_address += size;
  }

  //kprint("kmalloc now: new addr -> ");
  //kprint_hex(returnAdrr);
  //kprint(", phys addr -> ");
  
  //if(phys) kprint_hex(*phys);
  //else kprint_hex(0);

  //kprintln("");
  
  return returnAdrr;
}

uint32_t kmalloc_align(uint32_t size) {
  return _kmalloc(size, 1, 0);
}

uint32_t kmalloc_phys(uint32_t size, uint32_t *phys) {
  return _kmalloc(size, 0, phys);
}

uint32_t kmalloc_align_phys(uint32_t size, uint32_t *phys) {
  return _kmalloc(size, 1, phys);
}

uint32_t kmalloc(uint32_t size) {
  return _kmalloc(size, 0, 0);
}

void kfree(void *p) {
  free(p, global_heap);
}

static void expand(uint32_t new_size, heap_t *heap) {
  kprint("HAD TO EXPANED TO: ");
  kprint_hex(new_size);
  kprintln("");

  if (new_size & 0x00000FFF) {
    new_size &= 0xFFFFF000;
    new_size += 0x1000;
  }

  uint32_t old_size = heap->end_addr - heap->start_addr;
  uint32_t i = old_size;
  while (i < new_size) {
    alloc_frame(get_page(heap->start_addr + i, 1, kernel_directory), (heap->supervisor) ? 1 : 0,
      (heap->readonly) ? 1 : 0);
    i += 0x1000;
  }

  heap->end_addr = heap->start_addr + new_size;
}

static uint32_t contract(uint32_t new_size, heap_t *heap) {
  if ((new_size & 0x00000FFF) != 0) {
    new_size &= 0xFFFFF000;
    new_size += 0x1000;
  }

  if (new_size < HEAP_MIN_SIZE) new_size = HEAP_MIN_SIZE;

  uint32_t old_size = heap->end_addr - heap->start_addr;
  uint32_t i = old_size - 0x1000;
  while (new_size < i) {
    free_frame(get_page(heap->start_addr + i, 0, kernel_directory));
    i -= 0x1000;
  }

  heap->end_addr = heap->start_addr + new_size;

  return new_size;
}

static int32_t find_smallest_hole(uint32_t size, uint8_t align, heap_t *heap) {
  uint32_t iterator = 0;
  while(iterator < heap->index.size) {
    header_t *header = (header_t *) index_ordered_array(iterator, &heap->index);
    if (align > 0) {
      uint32_t location = (uint32_t)header;
      uint32_t offset = 0;
      if (((location+sizeof(header_t)) & 0xFFFFF000) != 0)
        offset = 0x1000 /* page size */  - (location+sizeof(header_t))%0x1000;

      uint32_t hole_size = (uint32_t)header->size - offset;

      if (hole_size >= (uint32_t) size) break;
    } else if (header->size >= size) {
      break;
    }

    iterator++;
  }

  if (iterator == heap->index.size) return -1;
  else return iterator;
}

static uint8_t header_less_than(type_t a, type_t b) {
  return (((header_t *)a)->size < ((header_t *)b)->size) ? 1 : 0;
}

heap_t *create_heap(uint32_t start, uint32_t end, uint32_t max, uint8_t supervisor, uint8_t readonly) {
  heap_t *heap = (heap_t *)kmalloc(sizeof(heap_t));

  heap->index = place_ordered_array((void *)start, HEAP_INDEX_SIZE, &header_less_than);
  start += sizeof(type_t) * HEAP_INDEX_SIZE;

  if (start & 0x00000FFF) {
    start &= 0xFFFFF000;
    start += 0x1000;
  }

  heap->start_addr = start;
  heap->end_addr = end;
  heap->max_addr = max;
  heap->supervisor = supervisor;
  heap->readonly = readonly;

  header_t *hole = (header_t *)start;
  hole->size = end - start;
  hole->magic = HEAP_MAGIC;
  hole->is_hole = 1;

  insert_ordered_array((void *)hole, &heap->index);

  return heap;
}

void *alloc(uint32_t size, uint8_t page_align, heap_t *heap)
{

    // Make sure we take the size of header/footer into account.
    uint32_t new_size = size + sizeof(header_t) + sizeof(footer_t);
    // Find the smallest hole that will fit.
    int32_t iterator = find_smallest_hole(new_size, page_align, heap);

    if (iterator == -1) // If we didn't find a suitable hole
    {
        // Save some previous data.
        uint32_t old_length = heap->end_addr - heap->start_addr;
        uint32_t old_end_addr = heap->end_addr;

        // We need to allocate some more space.
        expand(old_length+new_size, heap);
        int32_t new_length = heap->end_addr-heap->start_addr;

        // Find the endmost header. (Not endmost in size, but in location).
        iterator = 0;
        // Vars to hold the index of, and value of, the endmost header found so far.
        int32_t idx = -1; uint32_t value = 0x0;
        while (iterator < (int32_t)heap->index.size)
        {
            uint32_t tmp = (uint32_t)index_ordered_array(iterator, &heap->index);
            if (tmp > value)
            {
                value = tmp;
                idx = iterator;
            }
            iterator++;
        }

        // If we didn't find ANY headers, we need to add one.
        if (idx == -1)
        {
            header_t *header = (header_t *)old_end_addr;
            header->magic = HEAP_MAGIC;
            header->size = new_length - old_length;
            header->is_hole = 1;
            footer_t *footer = (footer_t *) (old_end_addr + header->size - sizeof(footer_t));
            footer->magic = HEAP_MAGIC;
            footer->header = header;
            insert_ordered_array((void*)header, &heap->index);
        }
        else
        {
            // The last header needs adjusting.
            header_t *header = index_ordered_array(idx, &heap->index);
            header->size += new_length - old_length;
            // Rewrite the footer.
            footer_t *footer = (footer_t *) ( (uint32_t)header + header->size - sizeof(footer_t) );
            footer->header = header;
            footer->magic = HEAP_MAGIC;
        }
        // We now have enough space. Recurse, and call the function again.
        return alloc(size, page_align, heap);
    }

    header_t *orig_hole_header = (header_t *)index_ordered_array(iterator, &heap->index);
    uint32_t orig_hole_pos = (uint32_t)orig_hole_header;
    uint32_t orig_hole_size = orig_hole_header->size;
    // Here we work out if we should split the hole we found into two parts.
    // Is the original hole size - requested hole size less than the overhead for adding a new hole?
    if (orig_hole_size-new_size < sizeof(header_t)+sizeof(footer_t))
    {
        // Then just increase the requested size to the size of the hole we found.
        size += orig_hole_size-new_size;
        new_size = orig_hole_size;
    }

    // If we need to page-align the data, do it now and make a new hole in front of our block.
    if (page_align && orig_hole_pos&0xFFFFF000)
    {
        uint32_t new_location   = orig_hole_pos + 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(header_t);
        header_t *hole_header = (header_t *)orig_hole_pos;
        hole_header->size     = 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(header_t);
        hole_header->magic    = HEAP_MAGIC;
        hole_header->is_hole  = 1;
        footer_t *hole_footer = (footer_t *) ( (uint32_t)new_location - sizeof(footer_t) );
        hole_footer->magic    = HEAP_MAGIC;
        hole_footer->header   = hole_header;
        orig_hole_pos         = new_location;
        orig_hole_size        = orig_hole_size - hole_header->size;
    }
    else
    {
        // Else we don't need this hole any more, delete it from the index.
        remove_ordered_array(iterator, &heap->index);
    }

    // Overwrite the original header...
    header_t *block_header  = (header_t *)orig_hole_pos;
    block_header->magic     = HEAP_MAGIC;
    block_header->is_hole   = 0;
    block_header->size      = new_size;
    // ...And the footer
    footer_t *block_footer  = (footer_t *) (orig_hole_pos + sizeof(header_t) + size);
    block_footer->magic     = HEAP_MAGIC;
    block_footer->header    = block_header;

    // We may need to write a new hole after the allocated block.
    // We do this only if the new hole would have positive size...
    if (orig_hole_size - new_size > 0)
    {
        header_t *hole_header = (header_t *) (orig_hole_pos + sizeof(header_t) + size + sizeof(footer_t));
        hole_header->magic    = HEAP_MAGIC;
        hole_header->is_hole  = 1;
        hole_header->size     = orig_hole_size - new_size;
        footer_t *hole_footer = (footer_t *) ( (uint32_t)hole_header + orig_hole_size - new_size - sizeof(footer_t) );
        if ((uint32_t)hole_footer < heap->end_addr)
        {
            hole_footer->magic = HEAP_MAGIC;
            hole_footer->header = hole_header;
        }
        // Put the new hole in the index;
        insert_ordered_array((void*)hole_header, &heap->index);
    }
    
    // ...And we're done!
    return (void *) ( (uint32_t)block_header+sizeof(header_t) );
}

void free(void *p, heap_t *heap) {
   if (p == 0) return;

  header_t *header = (header_t *)((uint32_t)p - sizeof(header_t) );
  footer_t *footer = (footer_t *)((uint32_t)header + header->size - sizeof(footer_t));

  header->is_hole = 1;

  char do_add = 1;

  footer_t *test_footer = (footer_t*)((uint32_t)header - sizeof(footer_t) );
  if (test_footer->magic == HEAP_MAGIC && test_footer->header->is_hole == 1) {
    uint32_t cache_size = header->size; // Cache our current size.
    header = test_footer->header;     // Rewrite our header with the new one.
    footer->header = header;          // Rewrite our footer to point to the new header.
    header->size += cache_size;       // Change the size.
    do_add = 0;                       // Since this header is already in the index, we don't want to add it again.
  }

  header_t *test_header = (header_t*)((uint32_t)footer + sizeof(footer_t));
  if (test_header->magic == HEAP_MAGIC && test_header->is_hole) {
    header->size += test_header->size; // Increase our size.
    test_footer = (footer_t*)((uint32_t)test_header + // Rewrite it's footer to point to our header.
                                    test_header->size - sizeof(footer_t) );
    footer = test_footer;
    
    uint32_t iterator = 0;
    while((iterator < heap->index.size) &&
      (index_ordered_array(iterator, &heap->index) != (type_t)test_header)) iterator++;

    remove_ordered_array(iterator, &heap->index);
  }

  if(((uint32_t)footer + sizeof(footer_t)) == heap->end_addr) {
    uint32_t old_length = heap->end_addr-heap->start_addr;
    uint32_t new_length = contract((uint32_t)header - heap->start_addr, heap);
    
    if (header->size - (old_length-new_length) > 0) {
      header->size -= old_length-new_length;
      footer = (footer_t*)((uint32_t)header + header->size - sizeof(footer_t));
      footer->magic = HEAP_MAGIC;
      footer->header = header;
    } else {
      uint32_t iterator = 0;
      while((iterator < heap->index.size) &&
        (index_ordered_array(iterator, &heap->index) != (type_t)test_header) ) iterator++;
      
      if (iterator < heap->index.size) remove_ordered_array(iterator, &heap->index);
    }
  }

  if (do_add == 1) insert_ordered_array((void*)header, &heap->index);
}
