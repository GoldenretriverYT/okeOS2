#pragma once
#include <stdint.h>

#define arraylen(array) (sizeof(array) / sizeof(array[0]))
#define BOCHS_BREAK asm volatile ("xchg %bx, %bx")