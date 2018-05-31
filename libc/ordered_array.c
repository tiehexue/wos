#include "ordered_array.h"

#include "../kernel/heap.h"
#include "../libc/mem.h"

uint8_t standard_lessthan_predicate(type_t a, type_t b) {
  return (a < b) ? 1 : 0;
}

ordered_array_t create_ordered_array(uint32_t max_size, lessthan_predicate_t less_than) {
  
  void *addr = kmalloc(max_size * sizeof(type_t));
  
  return place_ordered_array(addr, max_size, less_than);
}

ordered_array_t place_ordered_array(void *addr, uint32_t max_size, lessthan_predicate_t less_than) {
  ordered_array_t new;
  new.array = (type_t *)addr;
  memory_set((uint8_t *)new.array, 0, max_size * sizeof(type_t));
  new.size = 0;
  new.max_size = max_size;
  new.less_than = less_than;

  return new;
}

void destroy_ordered_array(ordered_array_t *array) {
  kfree(array);
}

void insert_ordered_array(type_t item, ordered_array_t *array) {
  uint32_t iterator = 0;
  while (iterator < array->size && array->less_than(array->array[iterator], item)) iterator++;

  if (iterator == array->size) array->array[array->size++] = item;
  else {
    type_t tmp = array->array[iterator];
    array->array[iterator] = item;
    while(iterator < array->size) {
      iterator ++;
      type_t next = array->array[iterator];
      array->array[iterator] = tmp;
      tmp = next;
    }

    array->size++;
  }
}

type_t index_ordered_array(uint32_t index, ordered_array_t *array) {
  return array->array[index];
}

void remove_ordered_array(uint32_t index, ordered_array_t *array) {
  while (index < array->size) {
    array->array[index] = array->array[index + 1];
    index ++;
  }
  array->size--;
}