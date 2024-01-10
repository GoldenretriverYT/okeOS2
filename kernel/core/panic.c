#include "panic.h"

void panic(char* str) {
    write_serial_string("PANIC: ");
    write_serial_string(str);
    write_serial_string("\nhalting yo goofy ahh kernel\n");

    // This is gonna make it appear twice in the serial output, but in case this wouldnt work we have the above code as a backup
    kprintf("PANIC: %s\nhalting yo goofy ahh kernel\n", str);
    
    for(;;)
        asm("hlt");
}