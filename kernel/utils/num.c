#include "num.h"

void itoa_i64(int64_t value, char* str, int base) {
    if (base < 2 || base > 36) {
        *str = '\0';
        return;
    }

    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    int64_t tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];
    } while (value);

    if (tmp_value < 0) {
        *ptr++ = '-';
    }

    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

void itoa_i32(int32_t value, char* str, int base) {
    itoa_i64(value, str, base);
}

void itoa_i16(int16_t value, char* str, int base) {
    itoa_i64(value, str, base);
}

void itoa_i8(int8_t value, char* str, int base) {
    itoa_i64(value, str, base);
}

void itoa_u64(uint64_t value, char* str, int base) {
    if (base < 2 || base > 36) {
        *str = '\0';
        return;
    }

    char* ptr = str;
    char* ptr1;
    uint64_t tmp_value;

    do {
        tmp_value = value % base;
        value /= base;
        if (tmp_value < 10) {
            *ptr++ = '0' + tmp_value;
        } else {
            *ptr++ = 'a' + (tmp_value - 10);
        }
    } while (value);

    *ptr-- = '\0';

    // Reverse the string
    for (ptr1 = str; ptr1 < ptr; ptr1++, ptr--) {
        char tmp_char = *ptr;
        *ptr = *ptr1;
        *ptr1 = tmp_char;
    }
}


void itoa_u32(uint32_t value, char* str, int base) {
    itoa_u64(value, str, base);
}

void itoa_u16(uint16_t value, char* str, int base) {
    itoa_u64(value, str, base);
}

void itoa_u8(uint8_t value, char* str, int base) {
    itoa_u64(value, str, base);
}