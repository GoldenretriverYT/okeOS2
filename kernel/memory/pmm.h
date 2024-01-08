#pragma once
#include <global.h>
#include <limine.h>
#include "../kernel.h"
#include "../io/serial.h"
#include "../core/panic.h"

#define PMM_DEBUG_ENABLED

#ifdef PMM_DEBUG_ENABLED
    #define PMM_DEBUG(str) write_serial_string(str);
    #define PMM_DEBUG_U64X(u64) write_serial_u64(u64, 16);
    #define PMM_DEBUG_U64(u64) write_serial_u64(u64, 10);
#else
    #define PMM_DEBUG(str)
    #define PMM_DEBUG_U64X(u64)
    #define PMM_DEBUG_U64(u64)
#endif

#define PMM_PAGE_SIZE 4096

#define PHYS2HIHA(addr) ((void*)(addr) + hhdm_request.response->offset)
#define HIHA2PHYS(addr) ((void*)(addr) - hhdm_request.response->offset)

void init_pmm(struct limine_memmap_entry** memmap, uint64_t memmap_entries);
void* pmm_alloc(uint64_t pages);
void* pmm_alloc_purpose(uint64_t pages, char purpose[5]);
void pmm_free(void* addr, uint64_t pages);