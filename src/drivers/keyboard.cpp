#include "keyboard.hpp"
#include "../cpu/ports.hpp"
#include "screen.hpp"
#include "../libc/string.hpp"
#include "../cpu/interrupt.hpp"

#include <stdint.h>

#define BACKSPACE 0x0E
#define ENTER 0x1C

static char key_buffer[256];

#define SC_MAX 57

const char *sc_name[] = { "ERROR", "Esc", "1", "2", "3", "4", "5", "6", 
  "7", "8", "9", "0", "-", "=", "Backspace", "Tab", "Q", "W", "E", 
  "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Lctrl", 
  "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`", 
  "LShift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".", 
  "/", "RShift", "Keypad *", "LAlt", "Spacebar"};
const char sc_ascii[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
  '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 
  'U', 'I', 'O', 'P', '[', ']', '?', '?', 'A', 'S', 'D', 'F', 'G', 
  'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V', 
  'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '};

void user_input(char *input) {
  if (strcmp(input, "END") == 0) {
    kprintln("Stopping the CPU, bye!");
    asm volatile("hlt");
  }

  kprint("You typed: ");
  kprintln(input);
  kprint("shell$ ");
}


static void keyboard_callback(interrupt::syscall_regs *regs, void *data) {
  uint8_t scancode = in_byte(0x60);
    
  if (scancode > SC_MAX) return;
  if (scancode == BACKSPACE) {
    // shell.
    if(kprint_backspace() == 1) backspace(key_buffer);
  } else if (scancode == ENTER) {
    kprint("\n");
    user_input(key_buffer); /* kernel-controlled function */
    key_buffer[0] = '\0';
  } else {
    char letter = sc_ascii[(int)scancode];
    char str[2] = {letter, '\0'};
    append(key_buffer, letter);
    kprint(str);
  }
}

void init_keyboard() {
  interrupt::register_irq_handler(1, keyboard_callback, 0); 
}
