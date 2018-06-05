#ifndef TASK_H
#define TASK_H

#include "../cpu/page.h"

typedef struct task {
  uint32_t id;
  uint32_t esp, ebp;
  uint32_t eip;
  page_directory_t *page_directory;
  struct task *next;
} task_t;

void init_tasking();

void switch_task();

uint32_t fork();

void move_stack(void *new_stack_start, uint32_t size);

uint32_t getpid();

#endif
