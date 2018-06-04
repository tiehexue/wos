#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

typedef struct multiboot_aout_symbol_table {
  uint32_t tabsize;
  uint32_t strsize;
  uint32_t addr;
  uint32_t reserved;
} __attribute__((packed)) multiboot_aout_symbol_table_t;

typedef struct multiboot_elf_section_header_table {
  uint32_t num;
  uint32_t size;
  uint32_t addr;
  uint32_t shndx;
} __attribute__((packed)) multiboot_elf_section_header_table_t;

typedef struct multiboot_info {
  uint32_t flags;
     
  uint32_t mem_lower;
  uint32_t mem_upper;
     
  uint32_t boot_device;
     
  uint32_t cmdline;
     
  uint32_t mods_count;
  uint32_t mods_addr;
     
  union {
    multiboot_aout_symbol_table_t aout_sym;
    multiboot_elf_section_header_table_t elf_sec;
  } u;
     
  uint32_t mmap_length;
  uint32_t mmap_addr;
     
  uint32_t drives_length;
  uint32_t drives_addr;
     
  uint32_t config_table;
     
  uint32_t boot_loader_name;
     
  uint32_t apm_table;
     
  uint32_t vbe_control_info;
  uint32_t vbe_mode_info;
  uint16_t vbe_mode;
  uint16_t vbe_interface_seg;
  uint16_t vbe_interface_off;
  uint16_t vbe_interface_len;
     
  uint64_t framebuffer_addr;
  uint32_t framebuffer_pitch;
  uint32_t framebuffer_width;
  uint32_t framebuffer_height;
  uint8_t framebuffer_bpp;

  uint8_t framebuffer_type;

  union {
    struct {
      uint32_t framebuffer_palette_addr;
      uint16_t framebuffer_palette_num_colors;
    };
    struct {
      uint8_t framebuffer_red_field_position;
      uint8_t framebuffer_red_mask_size;
      uint8_t framebuffer_green_field_position;
      uint8_t framebuffer_green_mask_size;
      uint8_t framebuffer_blue_field_position;
      uint8_t framebuffer_blue_mask_size;
    };
  };
} __attribute__((packed)) multiboot_t;

typedef struct multiboot_mod_list {
       /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
  uint32_t mod_start;
  uint32_t mod_end;
     
       /* Module command line */
  uint32_t cmdline;
     
       /* padding to take it to 16 bytes (must be zero) */
  uint32_t pad;
} __attribute__((packed)) multiboot_module_t;

#endif
