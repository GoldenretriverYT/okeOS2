#pragma once
#include <stdint.h>
#include <stddef.h>
#include "../panic.h"
#include "../../utils/mem.h"

#define _ReservedVec_DivideByZero 0
#define _ReservedVec_Debug 1
#define _ReservedVec_NonMaskableInterrupt 2
#define _ReservedVec_Breakpoint 3
#define _ReservedVec_Overflow 4
#define _ReservedVec_BoundRangeExceeded 5
#define _ReservedVec_InvalidOpcode 6
#define _ReservedVec_DeviceNotAvailable 7
#define _ReservedVec_DoubleFault 8
#define _ReservedVec_CoprocessorSegmentOverrun 9
#define _ReservedVec_InvalidTSS 10
#define _ReservedVec_SegmentNotPresent 11
#define _ReservedVec_StackSegmentFault 12
#define _ReservedVec_GeneralProtectionFault 13
#define _ReservedVec_PageFault 14
#define _ReservedVec_Reserved15 15
#define _ReservedVec_x87FloatingPointException 16
#define _ReservedVec_AlignmentCheck 17
#define _ReservedVec_MachineCheck 18
#define _ReservedVec_SIMDException 19

typedef struct InterruptDescriptor
{
    uint16_t addressLow;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  flags;
    uint16_t addressMid;
    uint32_t addressHigh;
    uint32_t reserved;
} __attribute__((packed)) InterruptDescriptor;

typedef struct IDTR
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) IDTR;

extern InterruptDescriptor idt[256];

void setIdtEntry(uint8_t vector, void* handler, uint8_t dpl);
void loadIDT(void* idtAddr);
void initInts();

typedef struct CPUStatus
{
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t vectorNumber;
    uint64_t errorCode;

    uint64_t iret_rip;
    uint64_t iret_cs;
    uint64_t iret_flags;
    uint64_t iret_rsp;
    uint64_t iret_ss;
} __attribute__((packed)) CPUStatus;

extern char fixedPageFaultString[256];

void __intsAppend(const char** str , uint64_t* strOffset);
extern void interruptDispatch(CPUStatus* ctx);
const char* getPageFaultString(uint64_t errorCode);

extern char vector_0_handler[];