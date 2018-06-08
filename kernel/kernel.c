#include "../drivers/screen.h"
#include "../cpu/isr.h"
#include "../cpu/gdt.h"
#include "../cpu/timer.h"
#include "../cpu/page.h"
#include "../libc/string.h"
#include "../libc/memory.h"
#include "../vfs/fs.h"
#include "../vfs/initrd.h"

#include "multiboot.h"
#include "heap.h"

#include <stdint.h>

extern uint32_t placement_address;

uint32_t multiboot_mem_upper = 0;
uint32_t initial_esp = 0;
uint32_t placement_address = 0;

static void readInitrd() {
  if (fs_root) {
    int i = 0;
    struct dirent *node = 0;
    while((node = readdir_fs(fs_root, i)) != 0) {
      kprint("Found file ");
      kprint(node->name);
      fs_node_t *fsnode = finddir_fs(fs_root, node->name);

      if ((fsnode->flags&0x7) == FS_DIRECTORY) {
        kprintln(" (directory)");
      } else {
        kprint(" contents: \"");
        uint8_t *buf = kmalloc(fsnode->length + 1);
        read_fs(fsnode, 0, fsnode->length, buf);
        buf[fsnode->length] = '\0';

        kprint((char *)buf);
        kprintln("\"");
      }
      i++;
    }
  }
}

void user_input(char *input) {
  if (strcmp(input, "END") == 0) {
    kprintln("Stopping the CPU, bye!");
    asm volatile("hlt");
  } else if (strcmp(input, "PAGE") == 0) {
    uint32_t phys_addr;
    uint32_t page = kmalloc_physical(0x40000000, &phys_addr);

    kprint("Page: ");
    kprintln_hex(page);
    kprint(", physical address: ");
    kprintln_hex(phys_addr);
  } else if (strcmp(input, "INITRD") == 0) {
    readInitrd();
  }

  kprint("You typed: ");
  kprintln(input);
  kprint("shell$ ");
}


void kernel_main(multiboot_t *mboot_ptr, uint32_t esp) {

  multiboot_mem_upper = mboot_ptr->mem_upper * 1024;
  initial_esp = esp;

  clear_screen();
  kprint("Hello, I am happy to see you.\n");

  init_gdt();
  isr_install();

  //asm volatile("int $1");
  //asm volatile("int $2");
  //asm volatile("int $3");
  //asm volatile("int $4");
  //asm volatile("int $13");
  //asm volatile("int $14");

  // int a = 3 / 0; // keep interrupting ?

  irq_install();

  //asm volatile("int $14");

  placement_address = *((uint32_t*)(mboot_ptr->mods_addr + 4));

  initialise_paging();

  fs_root = initialise_initrd(*((uint32_t *)mboot_ptr->mods_addr));
  kprintln("Initrd file initialized.");
  readInitrd();

  kprint("shell$ ");
  for(;;);
}
