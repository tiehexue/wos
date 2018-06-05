#include "../drivers/screen.h"
#include "../cpu/isr.h"
#include "../cpu/gdt.h"
#include "../cpu/timer.h"
#include "../cpu/page.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include "../vfs/fs.h"
#include "../vfs/initrd.h"
#include "multiboot.h"
#include "heap.h"

#include <stdint.h>

extern void paget_test(char *msg);
extern uint32_t placement_address;

uint32_t multiboot_mem_upper = 0;

void kernel_main(multiboot_t *mboot_ptr) {

  multiboot_mem_upper = mboot_ptr->mem_upper * 1024;

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

  // Initialise the initial ramdisk, and set it as the filesystem root.
 
  placement_address = *(uint32_t *)(mboot_ptr->mods_addr + 4);

  init_paging();

  fs_root = initialise_initrd(*((uint32_t *)mboot_ptr->mods_addr));
  kprintln("Initrd file initialized.");
  kprint("shell$ ");
  for(;;);
}

void user_input(char *input) {
  if (strcmp(input, "END") == 0) {
    kprintln("Stopping the CPU, bye!");
    asm volatile("hlt");
  } else if (strcmp(input, "PAGE") == 0) {
    uint32_t phys_addr;
    uint32_t page = kmalloc_phys(0x40000000, &phys_addr);
    char page_str[16] = "";
    int2hex(page, page_str);
    char phys_str[16] = "";
    int2hex(phys_addr, phys_str);
    kprint("Page: ");
    kprint(page_str);
    kprint(", physical address: ");
    kprintln(phys_str);
  } else if (strcmp(input, "INITRD") == 0) {
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

  kprint("You typed: ");
  kprintln(input);
  kprint("shell$ ");
}
