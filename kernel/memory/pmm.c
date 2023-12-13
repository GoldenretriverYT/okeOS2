#include "pmm.h"

// The bitmap allocator bitmap. 1 means allocated, 0 means free.
uint8_t* bitmap;

// Size of bitmap in bytes.
uint64_t bitmap_size;

// Amount of pages held by the bitmap.
uint64_t bitmap_pages;

// The current index of the bitmap allocator search, used to speed up the search.
uint64_t bitmap_index;

void pmm_set_bitmap(uint64_t index, uint8_t value) {
    uint64_t byte_index = index / 8;
    uint8_t bit_index = index % 8;

    if (value) {
        bitmap[byte_index] |= (1 << bit_index);
    } else {
        bitmap[byte_index] &= ~(1 << bit_index);
    }
}

uint8_t pmm_get_bitmap(uint64_t index) {
    uint64_t byte_index = index / 8;
    uint8_t bit_index = index % 8;

    return (bitmap[byte_index] >> bit_index) & 1;
}

void init_pmm(struct limine_memmap_entry** memmap, uint64_t memmap_entries) {
    uint64_t highestAddr = 0;

    for (uint64_t i = 0; i < memmap_entries; i++) {
        struct limine_memmap_entry* entry = memmap[i];

        write_serial_string("Entry: Base 0x");
        write_serial_u64(entry->base, 16);
        write_serial_string(" Length 0x");
        write_serial_u64(entry->length, 16);
        write_serial_string(" (");
        write_serial_u64(entry->length / 1024 / 1024, 10);
        write_serial_string(" MiB)");
        write_serial_string(" Type ");
        write_serial_u64(entry->type, 10);
        write_serial_string("\n");

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (entry->base + entry->length > highestAddr) {
            highestAddr = entry->base + entry->length;
        }
    }

    // Find the earliest free area that is at least as large as the bitmap.
    bitmap_pages = highestAddr / PMM_PAGE_SIZE;
    bitmap_size = bitmap_pages / 8 + 1;

    for (uint64_t i = 0; i < memmap_entries; i++) {
        struct limine_memmap_entry* entry = memmap[i];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (entry->length >= bitmap_size) {
            bitmap = PHYS2HIHA(entry->base);
            memset(bitmap, 0xFF, bitmap_size);
            break;
        }
    }

    // Mark the bitmap as allocated.
    for (uint64_t i = 0; i < bitmap_pages; i++) {
        pmm_set_bitmap(i, 1);
    }

    // Mark free areas as free.
    for (uint64_t i = 0; i < memmap_entries; i++) {
        struct limine_memmap_entry* entry = memmap[i];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        uint64_t start = entry->base / PMM_PAGE_SIZE;
        uint64_t end = (entry->base + entry->length) / PMM_PAGE_SIZE;

        for (uint64_t j = start; j < end; j++) {
            pmm_set_bitmap(j, 0);
        }
    }
}

void* pmm_alloc(uint64_t pages) {
    if(pages > 1024 * 256) {
        write_serial_string("PMM: Tried to allocate ");
        write_serial_u64(pages, 10);
        write_serial_string(" pages at once, you may only allocate up to 1 GiB at once.\n");
        return NULL;
    }

    PMM_DEBUG("PMM: Allocating ");
    PMM_DEBUG_U64(pages);
    PMM_DEBUG(" pages (");
    PMM_DEBUG_U64(pages * PMM_PAGE_SIZE / 1024);
    PMM_DEBUG(" KiB)...\n");

    for (; bitmap_index < bitmap_pages; bitmap_index++) {
        if (pmm_get_bitmap(bitmap_index)) {
            continue;
        }

        uint64_t free_pages = 0;

        for (uint64_t i = bitmap_index; i < bitmap_pages; i++) {
            if (pmm_get_bitmap(i)) {
                break;
            }

            free_pages++;

            if (free_pages == pages) {
                uint64_t addr = i - pages + 1;
                for (uint64_t j = addr; j < addr + pages; j++) {
                    pmm_set_bitmap(j, 1);
                }

                PMM_DEBUG("PMM: Allocated ");
                PMM_DEBUG_U64(pages);
                PMM_DEBUG(" pages at 0x");
                PMM_DEBUG_U64X(addr * PMM_PAGE_SIZE);
                PMM_DEBUG(" (");
                PMM_DEBUG_U64(pages * PMM_PAGE_SIZE / 1024);
                PMM_DEBUG(" KiB)\n");

                return (void*)(addr * PMM_PAGE_SIZE);
            }
        }
    }

    write_serial_string("PMM: Failed to allocate ");
    write_serial_u64(pages, 10);
    write_serial_string(" pages. (Out of memory); PMM had a total of ");
    write_serial_u64(bitmap_pages, 10);
    write_serial_string(" pages.\n");
    return NULL;
}

void pmm_free(void* addr, uint64_t pages) {
    uint64_t index = (uint64_t)(addr) / PMM_PAGE_SIZE;

    for (uint64_t i = index; i < index + pages; i++) {
        pmm_set_bitmap(i, 0);
    }

    if (index < bitmap_index) {
        bitmap_index = index;
    }
}