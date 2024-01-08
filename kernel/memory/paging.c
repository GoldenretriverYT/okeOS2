#include "paging.h"

static uint64_t kvirt_addr;
static uint64_t kphys_addr;
static page_table_t* kernel_pml4;
static uint8_t dbg = 0;

uint64_t ensure_canonical_address(uint64_t addr) {
    if (addr & 0x800000000000) { // Check if bit 47 is 1
        return addr | 0xFFFF000000000000; // Set upper 16 bits to 1s
    }
    return addr & 0x0000FFFFFFFFFFFF; // Clear upper 16 bits
}


void init_paging(uint64_t _kvirt_addr, uint64_t _kphys_addr, struct limine_memmap_response* memmap_response, struct limine_framebuffer* fb)
{
    kvirt_addr = _kvirt_addr;
    kphys_addr = _kphys_addr;

    kprintf("Paging: kvirt_addr: %llx\n", kvirt_addr);
    kprintf("Paging: kphys_addr: %llx\n", kphys_addr);

    kprintf("Paging: Creating new PML4\n");
    kernel_pml4 = create_pml4();
    kprintf("Paging: Created new PML4, at 0x%llx\n", kernel_pml4);

    // Identity map the first 1 MiB
    kprintf("Paging: Identity mapping first 1 MiB\n");
    for(uint64_t i = 0; i < 0x100000; i += PAGE_SIZE) {
        map_page(kernel_pml4, i, i, VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE);
    }
    kprintf("Paging: Identity mapped first 1 MiB\n");

    // Map the kernel and other important stuff
    kprintf("Paging: Mapping kernel and other important stuff\n");
    for(int memmapIdx = 0; memmapIdx < memmap_response->entry_count; memmapIdx++) {
        struct limine_memmap_entry* entry = memmap_response->entries[memmapIdx];

        kprintf("Entry in map at addr %llx\n", (uint64_t)entry);

        //if(entry->type == LIMINE_MEMMAP_USABLE) continue;

        uint64_t start = entry->base;
        uint64_t end = entry->base + entry->length;

        kprintf("Entry: Base 0x%llx Length 0x%llx (", entry->base, entry->length);
        kprintf("%llu MiB) Type %llu\n", entry->length / 1024 / 1024, entry->type);

        //if((entry->base == 0xb0000000) && (entry->length = 0x10000000)) dbg = 1; else dbg = 0;
        for(uint64_t i = start; i < end; i += PAGE_SIZE) {
            uint64_t virt = PHYS2HIHA(i);

            if(entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES) {
                virt = kvirt_addr + (i - kphys_addr);
            }else {
            }

            map_page(kernel_pml4, i, virt, VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE);
        }
    }

    // For some odd reason, the framebuffer is NOT in the memory map despite LIMINE_MEMMAP_FRAMEBUFFER existing ?! So we have to map it manually
    kprintf("Paging: Mapping framebuffer\n");
    dbg = 1;
    for(uint64_t i = align_down((uint64_t)(fb->address), PAGE_SIZE); i < align_up((uint64_t)(fb->address) + fb->height * fb->pitch, PAGE_SIZE); i += PAGE_SIZE) {
        map_page(kernel_pml4, i, PHYS2HIHA(i), VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE);
    }
    dbg = 0;

    kprintf("Paging: Mapped kernel and other important stuff\n");

    // Switch to the new PML4
    kprintf("Paging: Switching to new PML4\n");
    switch_pml4(kernel_pml4);
    kprintf("Paging: Switched to new PML4\n");
}

page_table_t* create_pml4() {
    page_table_t* pml4 = (page_table_t*) PHYS2HIHA(pmm_alloc(1));
    memset(pml4, 0, PAGE_SIZE);
    return pml4;
}


void map_page(page_table_t* pml4, uint64_t physical_address, uint64_t virtual_address, uint64_t flags) {
    if(physical_address % PAGE_SIZE != 0) {
        write_serial_string("Paging: Tried to map a page with an unaligned physical address.\n");
        return;
    }

    if(virtual_address % PAGE_SIZE != 0) {
        write_serial_string("Paging: Tried to map a page with an unaligned virtual address.\n");
        return;
    }

    uint64_t pml4_index = (virtual_address >> 39) & 0x1FF;
    uint64_t pdpt_index = (virtual_address >> 30) & 0x1FF;
    uint64_t pd_index = (virtual_address >> 21) & 0x1FF;
    uint64_t pt_index = (virtual_address >> 12) & 0x1FF;

    page_table_t* pdpt;
    if(pml4->entries[pml4_index] & VMM_FLAG_PRESENT) {
        pdpt = (page_table_t*) PHYS2HIHA(pml4->entries[pml4_index] & 0xFFFFFFFFFF000);
    } else {
        pdpt = (page_table_t*) PHYS2HIHA(pmm_alloc_purpose(1, "PDPT"));
        memset(pdpt, 0, PAGE_SIZE);
        pml4->entries[pml4_index] = (uint64_t)(HIHA2PHYS(pdpt)) | VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE;
    }

    page_table_t* pd;
    if(pdpt->entries[pdpt_index] & VMM_FLAG_PRESENT) {
        pd = (page_table_t*) PHYS2HIHA(pdpt->entries[pdpt_index] & 0xFFFFFFFFFF000);
    } else {
        pd = (page_table_t*) PHYS2HIHA(pmm_alloc_purpose(1, "PD  "));
        memset(pd, 0, PAGE_SIZE);
        pdpt->entries[pdpt_index] = (uint64_t)(HIHA2PHYS(pd)) | VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE;
    }

    page_table_t* pt;
    if(pd->entries[pd_index] & VMM_FLAG_PRESENT) {
        pt = (page_table_t*) PHYS2HIHA(pd->entries[pd_index] & 0xFFFFFFFFFF000);
    } else {
        pt = (page_table_t*) PHYS2HIHA(pmm_alloc_purpose(1, "PT  "));
        memset(pt, 0, PAGE_SIZE);
        pd->entries[pd_index] = (uint64_t)(HIHA2PHYS(pt)) | VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE;
    }

    if(dbg) {
        kprintf("Paging: P:%llx -> V:%llx\n", physical_address, virtual_address);
        kprintf("Paging: PML4_INDEX: %llu PDPT_INDEX: %llu PD_INDEX: %llu PT_INDEX: %llu\n", pml4_index, pdpt_index, pd_index, pt_index);
        kprintf("PDPT: %llx PD: %llx PT: %llx\n", pdpt, pd, pt);
    }

    pt->entries[pt_index] = (uint64_t)(physical_address) | flags;
}

void switch_pml4(page_table_t* pml4) {
    asm volatile ("mov %0, %%cr3" : : "r" (HIHA2PHYS(pml4)));
}