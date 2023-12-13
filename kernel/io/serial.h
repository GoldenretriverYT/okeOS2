#pragma once
#include "io.h"
#include "../utils/num.h"

#define PORT 0x3f8          // COM1

#pragma region Utils, Initialization
int init_serial();
int serial_received();
char read_serial();
int is_transmit_empty();
#pragma endregion
 
#pragma region Writing to Serial
void write_serial(char a);
void write_serial_string(char* a);
void write_serial_u64(uint64_t a, int base);
void write_serial_i64(int64_t a, int base);
#pragma endregion