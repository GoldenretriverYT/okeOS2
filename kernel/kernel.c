#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "io/serial.h"
#include "core/gdt.h"
#include "core/ints/ints.h"

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Halt and catch fire function.
static void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {
    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Initialise the serial port.
    init_serial();
    
    write_serial_string("=== okeOS2 ===\nFramebuffer: ");
    write_serial_u64(framebuffer->width, 10);
    write_serial_string(" x ");
    write_serial_u64(framebuffer->height, 10);
    write_serial_string(" x ");
    write_serial_u64(framebuffer->bpp, 10);
    write_serial_string("\n");

    write_serial_string("Loading GDT...\n");
    load_gdt();
    write_serial_string("GDT loaded.\n");

    write_serial_string("Loading IDT...\n");
    loadIDT(&idt);
    write_serial_string("IDT loaded.\n");
    // We're done, just hang...
    hcf();
}