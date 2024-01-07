#pragma once
#include "../global.h"
#include "../term/kprintf.h"
#include "pmm.h"
#include "../limine.h"

#define PAGE_SIZE 0x1000
#define VMM_FLAG_PRESENT 0x1
#define VMM_FLAG_WRITABLE 0x2
#define VMM_FLAG_USER 0x4
#define VMM_FLAG_WRITETHROUGH 0x8
#define VMM_FLAG_NOCACHE 0x10
#define VMM_FLAG_ACCESSED 0x20
#define VMM_FLAG_DIRTY 0x40

typedef struct page_table_t {
    uint64_t entries[512];
} __attribute__((aligned(0x1000))) page_table_t;

/// @brief Initialise paging
/// @param kernel_phys_addr 
/// @param kernel_virt_addr 
void init_paging(uint64_t kernel_phys_addr, uint64_t kernel_virt_addr, struct limine_memmap_response* memmap_response);

/// @brief Create a new PML4
/// @return 
page_table_t* create_pml4();

/// @brief Maps a page within the specified PML4
/// @param pml4 The PML4 to map the page in
/// @param physical_address The physical address to map
/// @param virtual_address The virtual address to map
/// @param flags The flags to use
void map_page(page_table_t* pml4, uint64_t physical_address, uint64_t virtual_address, uint64_t flags);

/// @brief Unmaps a page within the specified PML4
/// @param pml4 The PML4 to unmap the page in
/// @param virtual_address The virtual address to unmap
void unmap_page(page_table_t* pml4, uint64_t virtual_address);

/// @brief Switches the PML4 in CR3 to the specified PML4
/// @param pml4 The PML4 to switch to
void switch_pml4(page_table_t* pml4);

/// @brief Gets the physical address of a page within the specified PML4
/// @param pml4 The PML4 to get the physical address from
/// @param virtual_address The virtual address to get the physical address of
/// @return The physical address of the page
void* get_page_physical(page_table_t* pml4, uint64_t virtual_address);

/// @brief Gets the virtual address of a page within the specified PML4
/// @param pml4 The PML4 to get the virtual address from
/// @param physical_address The physical address to get the virtual address of
/// @return The virtual address of the page
void* get_page_virtual(page_table_t* pml4, uint64_t physical_address);