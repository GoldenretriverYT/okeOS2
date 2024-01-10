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

    if (!bitmap) {
        panic("PMM: Failed to find a suitable area for the bitmap.\n");
        return;
    }else {
        write_serial_string("PMM: Found a suitable area for the bitmap at 0x");
        write_serial_u64((uint64_t)bitmap, 16);
        write_serial_string(" (");
        write_serial_u64(bitmap_size / 1024, 10);
        write_serial_string(" KiB). Bitmap_Pages = ");
        write_serial_u64(bitmap_pages, 10);
        write_serial_string("\n");
    }

    // Mark free areas as free.
    for (uint64_t i = 0; i < memmap_entries; i++) {
        struct limine_memmap_entry* entry = memmap[i];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        uint64_t start = entry->base / PMM_PAGE_SIZE;
        uint64_t end = (entry->base + entry->length) / PMM_PAGE_SIZE;

        write_serial_string("PMM: Marking pages ");
        write_serial_u64(start, 10);
        write_serial_string(" (0x");
        write_serial_u64(start * PMM_PAGE_SIZE, 16);
        write_serial_string(") to ");
        write_serial_u64(end, 10);
        write_serial_string(" (0x");
        write_serial_u64(end * PMM_PAGE_SIZE, 16);
        write_serial_string(") as free.\n");

        for (uint64_t j = start; j < end; j++) {
            pmm_set_bitmap(j, 0);
        }
    }

    // Mark the bitmap itself as allocated.
    for (uint64_t i = (uint64_t)HIHA2PHYS((uint64_t)bitmap) / PMM_PAGE_SIZE; i < ((uint64_t)HIHA2PHYS((uint64_t)bitmap) / PMM_PAGE_SIZE + bitmap_size / PMM_PAGE_SIZE)+1; i++) {
        write_serial_string("PMM: Marking bitmap page ");
        write_serial_u64(i, 10);
        write_serial_string(" (0x");
        write_serial_u64(i * PMM_PAGE_SIZE, 16);
        write_serial_string(") as allocated.\n");
        pmm_set_bitmap(i, 1);
    }
}

void* pmm_alloc(uint64_t pages) {
    return pmm_alloc_purpose(pages, "UKWN");
}

void* pmm_alloc_purpose(uint64_t pages, char purpose[5]) {
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
    PMM_DEBUG(" KiB) for purpose ");
    PMM_DEBUG(purpose);
    PMM_DEBUG("\n");

    for (; bitmap_index < bitmap_pages; bitmap_index++) {
        if (pmm_get_bitmap(bitmap_index)) {
            continue;
        }

        uint64_t free_pages = 0;

        for (; bitmap_index < bitmap_pages; bitmap_index++) {
            if (pmm_get_bitmap(bitmap_index)) {
                free_pages = 0;
                break;
            }

            free_pages++;

            if (free_pages == pages) {
                uint64_t addr = bitmap_index - pages + 1;
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

                bitmap_index++;

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