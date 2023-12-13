#pragma once
#include <stdint.h>
#include <global.h>
#include "panic.h"

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed)) IDTEntry;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) IDTR;

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) CPUState;

void init_idt();
void idt_set_entry(uint8_t index, uint64_t handler);

void disable_interrupts();
void enable_interrupts();

void set_handler(uint8_t vector, void (*handler)(CPUState* cpu));