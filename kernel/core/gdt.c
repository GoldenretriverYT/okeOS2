#include "gdt.h"

const uint64_t num_gdt_entries = 5;
uint64_t gdt_entries[5];

GDTR gdtr = 
{
    .limit = num_gdt_entries * sizeof(uint64_t) - 1,
    .address = (uint64_t)gdt_entries
};

void load_gdt() {
    gdt_entries[0] = 0;

    uint64_t kernel_code = 0;
    kernel_code |= 0b1011 << 8; //type of selector
    kernel_code |= 1 << 12; //not a system descriptor
    kernel_code |= 0 << 13; //DPL field = 0
    kernel_code |= 1 << 15; //present
    kernel_code |= 1 << 21; //long-mode segment

    gdt_entries[1] = kernel_code << 32;

    uint64_t kernel_data = 0;
    kernel_data |= 0b0011 << 8; //type of selector
    kernel_data |= 1 << 12; //not a system descriptor
    kernel_data |= 0 << 13; //DPL field = 0
    kernel_data |= 1 << 15; //present
    kernel_data |= 1 << 21; //long-mode segment
    gdt_entries[2] = kernel_data << 32;

    uint64_t user_code = kernel_code | (3 << 13);
    gdt_entries[3] = user_code;

    uint64_t user_data = kernel_data | (3 << 13);
    gdt_entries[4] = user_data;

    asm("lgdt %0" : : "m"(gdtr));

    flush_gdt();
}

void flush_gdt()
{
    asm volatile("\
        mov $0x10, %ax \n\
        mov %ax, %ds \n\
        mov %ax, %es \n\
        mov %ax, %fs \n\
        mov %ax, %gs \n\
        mov %ax, %ss \n\
        \n\
        pop %rdi \n\
        push $0x8 \n\
        push %rdi \n\
        lretq \n\
    ");
}