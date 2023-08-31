#include "ints.h"

InterruptDescriptor idt[256];

void setIdtEntry(uint8_t vector, void* handler, uint8_t dpl) {
    uint64_t handlerAddr = (uint64_t)handler;

    InterruptDescriptor* entry = &idt[vector];
    entry->addressLow = handlerAddr & 0xFFFF;
    entry->addressMid = (handlerAddr >> 16) & 0xFFFF;
    entry->addressHigh = handlerAddr >> 32;
    entry->selector = 0x8;
    //trap gate + present + DPL
    entry->flags = 0b1110 | ((dpl & 0b11) << 5) |(1 << 7);
    //ist disabled
    entry->ist = 0;
}

void loadIDT(void* idtAddr)
{
    IDTR idtReg;
    idtReg.limit = 0xFFF;
    idtReg.base = (uint64_t)idtAddr;
    IDTR* idtRegPtr = &idtReg;
    asm volatile("lidt %0" :: "m"(idtRegPtr));
}

void initInts() {
    for (uint64_t i = 0; i < 256; i++)
    setIdtEntry(i, (void*)vector_0_handler + (i * 16), 0);
}

extern void interruptDispatch(CPUStatus* ctx)
{
    switch (ctx->vectorNumber)
    {
        case _ReservedVec_GeneralProtectionFault:
            panic("General Protection Fault");
            break;
        case _ReservedVec_PageFault:
            panic((char*)getPageFaultString(ctx->errorCode));
            break;
        default:
            write_serial_string("Unexpected interrupt: ");
            write_serial_u64(ctx->vectorNumber, 10);
            panic("Unexpected interrupt.");
            break;
    }
}

char fixedPageFaultString[256];

void __intsAppend(const char** str , uint64_t* strOffset) {
    for(uint64_t i = 0; i < strlen(str); i++) {
        fixedPageFaultString[*strOffset + i] = (*str)[i];
    }

    *strOffset += strlen(str);
}

const char* getPageFaultString(uint64_t errorCode) {
    // Construct a string considering following bits in the bitfield
    // 0: Present - 0 = not present, 1 = present | Present means that the page table entry does exist
    // 1: Write - 0 = read, 1 = write | Write means that the page was written to
    // 2: User - 0 = supervisor, 1 = user | User means that the page was accessed in user mode
    // 3: Reserved bit violation - 0 = no violation, 1 = violation | Reserved bit violation means that a reserved bit was set to 1
    // 4: Instruction fetch - 0 = data access, 1 = instruction fetch | Instruction fetch means that the page was accessed by an instruction fetch
    // 5: Protection key violation - 0 = no violation, 1 = violation | Protection key violation means that the page was accessed with a protection key that does not match the key in the PTE
    // We dont care about the others

    uint64_t present = errorCode & 0b1;
    uint64_t write = (errorCode >> 1) & 0b1;
    uint64_t user = (errorCode >> 2) & 0b1;
    uint64_t reservedBitViolation = (errorCode >> 3) & 0b1;
    uint64_t instructionFetch = (errorCode >> 4) & 0b1;
    uint64_t protectionKeyViolation = (errorCode >> 5) & 0b1;

    uint64_t strOffset = 0;
    
    if(user == 0) {
        __intsAppend("Supervisor Process ", &strOffset);
    }else {
        __intsAppend("User Process ", &strOffset);
    }

    if(write == 0) {
        __intsAppend("read from ", &strOffset);
    }else {
        __intsAppend("write to ", &strOffset);
    }

    if(instructionFetch == 0) {
        __intsAppend("data ", &strOffset);
    }else {
        __intsAppend("instruction ", &strOffset);
    }

    if(present == 0) {
        __intsAppend("non-present page entry", &strOffset);
    }else {
        __intsAppend("and caused a protection fault", &strOffset);
    }

    return fixedPageFaultString;
}