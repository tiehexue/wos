#include "../drivers/screen.h"
#include "../cpu/isr.h"

void main() {
    clear_screen();
    kprint("Hello, I am happy to see you.\n");

    isr_install();

    __asm__ __volatile__("int $12");
    // int a = 3 / 0; // keep interrupting ?
    __asm__ __volatile__("int $3");
}
