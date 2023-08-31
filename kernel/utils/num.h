#pragma once
#include "stdint.h"
#include "stddef.h"

void itoa_i64(int64_t value, char* str, int base);
void itoa_i32(int32_t value, char* str, int base);
void itoa_i16(int16_t value, char* str, int base);
void itoa_i8(int8_t value, char* str, int base);

void itoa_u64(uint64_t value, char* str, int base);
void itoa_u32(uint32_t value, char* str, int base);
void itoa_u16(uint16_t value, char* str, int base);
void itoa_u8(uint8_t value, char* str, int base);