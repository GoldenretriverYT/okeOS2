#include "gdt.h"

extern void reload_segments();

uint64_t gdt[] = {
    0x0000000000000000, // Null descriptor

    // 64-bit Code Segment (kernel mode)
    0x00af9a000000ffff, // Base = 0x0000000000000000, Limit = 0xfffff

    // Data Segment (kernel mode)
    0x00af92000000ffff, // Base = 0x0000000000000000, Limit = 0xfffff

    // 64-bit Code Segment (user mode)
    0x00affa000000ffff, // Base = 0x0000000000000000, Limit = 0xfffff

    // Data Segment (user mode)
    0x00aff2000000ffff, // Base = 0x0000000000000000, Limit = 0xfffff
};


GDTR gdtr = {
    .size = (sizeof(uint64_t) * 5) - 1,
    .offset = (uint64_t)&gdt
};

void init_gdt() {
    asm volatile ("lgdt %0" :: "m"(gdtr) : "memory");
    reload_segments();
}