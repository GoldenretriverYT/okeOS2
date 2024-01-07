#include "kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "io/serial.h"
#include "utils/mem.h"
#include "core/gdt.h"
#include "core/idt.h"
#include "memory/pmm.h"
#include "term/term.h"
#include "term/kprintf.h"

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

volatile struct limine_kernel_address_request kernel_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

// Halt and catch fire function.
static void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

void irqTest(CPUState* cpu) {
    write_serial_string("IRQ 0x20\n");
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
    struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];
    struct limine_memmap_entry** memmap = memmap_request.response->entries;
    uint64_t memmap_entries = memmap_request.response->entry_count;

    hhdm.offset = hhdm_request.response->offset;

    // Initialise the serial port.
    init_serial();
    
    write_serial_string("=== okeOS2 ===\nFramebuffer: ");
    write_serial_u64(framebuffer->width, 10);
    write_serial_string(" x ");
    write_serial_u64(framebuffer->height, 10);
    write_serial_string(" x ");
    write_serial_u64(framebuffer->bpp, 10);
    write_serial_string("\n");

    write_serial_string("Kernel:\n  PhysBase: 0x");
    write_serial_u64(kernel_request.response->physical_base, 16);
    write_serial_string("\n");
    write_serial_string("  VirtBase: 0x");
    write_serial_u64(kernel_request.response->virtual_base, 16);
    write_serial_string("\n");
    write_serial_string("  HHDM: ");
    write_serial_u64(hhdm.offset, 16);
    write_serial_string("\n");


    write_serial_string("Initialising GDT...\n");
    init_gdt();
    write_serial_string("Initialised GDT.\n");

    write_serial_string("Initialising IDT...\n");
    init_idt();
    write_serial_string("Testing...\n");

    set_handler(0x20, irqTest);
    asm volatile ("int $0x40"); // IRQ 0x20 (32 (irq offset) + 0x20 (irq number))

    write_serial_string("Initialised IDT.\n");

    write_serial_string("Initialising PMM...\n");
    init_pmm(memmap, memmap_entries);
    write_serial_string("Initialised PMM.\n");

    write_serial_string("Alloc PMM test (4096, 16384)...\n");
    void* addr1 = pmm_alloc(4096 / PMM_PAGE_SIZE);
    void* addr2 = pmm_alloc(16384 / PMM_PAGE_SIZE);

    write_serial_string("Free PMM test (4096, 16384)...\n");
    pmm_free(addr1, 4096 / PMM_PAGE_SIZE);
    pmm_free(addr2, 16384 / PMM_PAGE_SIZE);

    write_serial_string("If it didn't crash, PMM at least kinda works.\n");

    write_serial_string("Initialising terminal...\n");
    term_init(framebuffer);
    kprintf("?.??? Terminal initialised with framebuffer at %llx\n", framebuffer->address);

    kprintf("?.??? Enabling Paging...\n");
    init_paging(kernel_request.response->virtual_base, kernel_request.response->physical_base, memmap_request.response);
    kprintf("?.??? Paging enabled\n");
    hcf();
}