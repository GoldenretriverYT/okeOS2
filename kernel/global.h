#pragma once
#include <stdint.h>

#define arraylen(array) (sizeof(array) / sizeof(array[0]))
#define BOCHS_BREAK asm volatile ("xchg %bx, %bx")
#define align_down(x, a) ((x) & ~((a) - 1))
#define align_up(x, a) align_down((x) + (a) - 1, (a))