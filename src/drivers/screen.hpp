#ifndef SCREEN_H
#define SCREEN_H

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0f
#define RED_ON_WHITE 0xf4

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

void clear_screen();

int kprint_backspace();

void kprint_at(const char *message, int col, int row);
void kprint(const char *message);
void kprintln(const char *message);

void kprint_hex(int a);
void kprint_int(int b);

void panic(const char *message);

#define kprintln_hex(x) kprint_hex(x);kprintln("");
#define kprintln_int(x) kprint_int(x);kprintln("");

#endif
