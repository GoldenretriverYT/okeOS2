#pragma once
#include <stdint.h>

extern const uint64_t num_gdt_entries;
extern uint64_t gdt_entries[];

typedef struct GDTR
{
    uint16_t limit;
    uint64_t address;
} __attribute__((packed)) GDTR;

extern GDTR gdtr;

void load_gdt();
void flush_gdt();