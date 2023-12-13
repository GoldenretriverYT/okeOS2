#include "idt.h"
extern void* idt_stub_table[];

static IDTEntry idt[256];
static IDTR idtr;
static void (*handlers[256])(CPUState* cpu);

void idt_set_entry(uint8_t index, uint64_t handler) {
    idt[index].offset_low = handler & 0xFFFF;
    idt[index].offset_mid = (handler >> 16) & 0xFFFF;
    idt[index].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[index].ist = 0;
    idt[index].selector = 0x08;
    idt[index].type_attr = 0x8E;
}

void init_idt() {
    for (int i = 0; i < 256; i++) {
        idt_set_entry(i, (uint64_t)idt_stub_table[i]);
    }

    idtr.limit = sizeof(IDTEntry) * 256 - 1;
    idtr.base = (uint64_t)&idt;

    asm volatile ("lidt %0" :: "m"(idtr) : "memory");
}

static const char* panic_msgs[20] = {
    "Unexpected: Divide by zero",
    "Unexpected: Debug",
    "Unexpected: Non-maskable interrupt",
    "Unexpected: Breakpoint",
    "Unexpected: Overflow",
    "Unexpected: Bound range exceeded",
    "Unexpected: Invalid opcode",
    "Unexpected: Device not available",
    "Unexpected: Double fault",
    "Unexpected: Coprocessor segment overrun",
    "Unexpected: Invalid TSS",
    "Unexpected: Segment not present",
    "Unexpected: Stack-segment fault",
    "Unexpected: General protection fault",
    "Unexpected: Page fault",
    "Unexpected: Reserved",
    "Unexpected: x87 floating-point exception",
    "Unexpected: Alignment check",
    "Unexpected: Machine check",
    "Unexpected: SIMD floating-point exception"
};

void handle_isr(CPUState* cpu) {
    if (cpu->int_no < arraylen(panic_msgs)) {
        panic(panic_msgs[cpu->int_no]);
    } else {
        panic("Unexpected: Unknown interrupt");
    }
}

void set_handler(uint8_t vector, void (*handler)(CPUState* cpu)) {
    write_serial_string("Setting handler for vector ");
    write_serial_u64(vector, 16);
    write_serial_string(" to ");
    write_serial_u64((uint64_t)handler, 16);
    write_serial_string("\n");

    handlers[vector] = handler;
}

void handle_irq(CPUState* cpu) {
    if (handlers[cpu->int_no - 32] != NULL && (uint64_t)handlers[cpu->int_no - 32] != 0) {
        handlers[cpu->int_no - 32](cpu);
    } else {
        panic("Unexpected: Unhandled IRQ");
    }
}