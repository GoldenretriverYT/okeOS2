#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct HHDM
{
    uint64_t offset;
} __attribute__((packed)) HHDM;

extern HHDM hhdm;

void *memcpy(void *dest, const void *src, size_t n);

void *memset(void *s, int c, size_t n);

void *memmove(void *dest, const void *src, size_t n);

int memcmp(const void *s1, const void *s2, size_t n);

int strcmp(const char *s1, const char *s2);

int strlen(const char *s);