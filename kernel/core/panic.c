#include "panic.h"

void panic(char* str) {
    write_serial_string("PANIC: ");
    write_serial_string(str);
    write_serial_string("\nhalting yo goofy ahh kernel\n");
    
    for(;;)
        asm("hlt");
}