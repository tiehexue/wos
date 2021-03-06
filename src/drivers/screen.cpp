#include "screen.hpp"
#include "../cpu/ports.hpp"
#include "../libc/memory.hpp"
#include "../libc/string.hpp"

static int get_cursor_offset() {
  out_byte(14, REG_SCREEN_CTRL);
  int offset = in_byte(REG_SCREEN_DATA) << 8;
  out_byte(15, REG_SCREEN_CTRL);
  offset += in_byte(REG_SCREEN_DATA);

  return offset * 2;
}

static void set_cursor_offset(int offset) {
  offset /= 2;
  out_byte(14, REG_SCREEN_CTRL);
  out_byte((unsigned char)(offset >> 8), REG_SCREEN_DATA);
  out_byte(15, REG_SCREEN_CTRL);
  out_byte((unsigned char)(offset & 0xff), REG_SCREEN_DATA);
}

static int get_offset(int col, int row) { return 2 * (row * MAX_COLS + col); }
static int get_offset_row(int offset) { return offset / (2 * MAX_COLS); }
static int get_offset_col(int offset) { return (offset - (get_offset_row(offset) * 2 * MAX_COLS)) / 2; }

static int print_char(char c, int col, int row, char attr) {
  unsigned char *vidmem = (unsigned char*) VIDEO_ADDRESS;
  if (!attr) attr = WHITE_ON_BLACK;

  if (col >= MAX_COLS || row >= MAX_ROWS) {
    vidmem[2 * (MAX_COLS) * (MAX_ROWS) - 2] = 'E';
    vidmem[2 * (MAX_COLS) * (MAX_ROWS) - 1] = RED_ON_WHITE;
    return get_offset(col, row);
  }

  int offset;
  if (col >= 0 && row >= 0) offset = get_offset(col, row);
  else offset = get_cursor_offset();

  if (c == '\n') {
    row = get_offset_row(offset);
    offset = get_offset(0, row + 1);
  } else {
    vidmem[offset] = c;
    vidmem[offset + 1] = attr;
    offset += 2;
  }

  if (offset >= MAX_COLS * MAX_ROWS * 2) {
    int i = 0;
    for (i = 1; i < MAX_ROWS; i ++)
      memcpy((uint8_t *)(get_offset(0, i - 1) + VIDEO_ADDRESS),
             (uint8_t *)(get_offset(0, i) + VIDEO_ADDRESS), MAX_COLS * 2);

    char *last_line = (char *)(get_offset(0, MAX_ROWS - 1) + VIDEO_ADDRESS);
    for (i = 0; i < MAX_COLS * 2; i++) last_line[i] = 0;

    offset -= 2 * MAX_COLS;
  }

  set_cursor_offset(offset);
  return offset;
}

void kprint_at(const char *message, int col, int row) {
  int offset;
  if (col >= 0 && row >= 0)
    offset = get_offset(col, row);
  else {
    offset = get_cursor_offset();
    row = get_offset_row(offset);
    col = get_offset_col(offset);
  }

  int i = 0;
  while(message[i] != 0) {
    offset = print_char(message[i++], col, row, WHITE_ON_BLACK);
    row = get_offset_row(offset);
    col = get_offset_col(offset);
  }
}

void kprint(const char *message) {
  kprint_at(message, -1, -1);
}

void kprintln(const char *message) {
  kprint(message);
  kprint("\n");
}

void kprint_hex(int a) {
  char str[16];
  itoh(a, str);
  kprint(str);
}

void kprint_int(int a) {
  char str[16];
  itoa(a, str);
  kprint(str);
}

int kprint_backspace() {
  int offset = get_cursor_offset() - 2;
  
  int col = get_offset_col(offset);

  // magic number: the shell prompt.
  if (col > 6) {
    int row = get_offset_row(offset);
    print_char(' ', col, row, WHITE_ON_BLACK);
    set_cursor_offset(offset);
    return 1;
  } else {
    return 0;
  }
}

void clear_screen() {
  int screen_size = MAX_COLS * MAX_ROWS;
  int i;
  char *screen = (char *)VIDEO_ADDRESS;

  for (i = 0; i < screen_size; i++) {
    screen[i * 2] = ' ';
    screen[i * 2 + 1] = WHITE_ON_BLACK;
  }

  set_cursor_offset(get_offset(0, 0));
}

void panic(char *message) {
  kprintln(message);
  kprintln("HALT");
  for(;;);
}
