#pragma once
#include "../global.h"
#include "../memory/pmm.h"
#include "../term/kprintf.h"
#include "../io/io.h"

#define ON_ACPI1 if(acpi_revision == 0) 
#define ON_ACPI2 if(acpi_revision != 0) 

typedef struct xsdp_t {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;      // deprecated since version 2.0

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__ ((packed)) xsdp_t;

typedef struct sdt_header_t {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__ ((packed)) sdt_header_t;

typedef struct rsdt_t {
    sdt_header_t header;
    uint32_t table_pointers[];
} __attribute__ ((packed)) rsdt_t;

typedef struct xsdt_t {
    sdt_header_t header;
    uint64_t table_pointers[];
} __attribute__ ((packed)) xsdt_t;

typedef struct generic_address_structure_t
{
    uint8_t address_space;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t access_size;
    uint64_t address;
} __attribute__ ((packed)) generic_address_structure_t;

typedef struct fadt_t
{
    struct sdt_header_t header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
 
    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t reserved;
 
    uint8_t preferred_power_management_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t pm1_event_length;
    uint8_t pm1_control_length;
    uint8_t pm2_control_length;
    uint8_t pm_timer_length;
    uint8_t gpe0_length;
    uint8_t gpe1_length;
    uint8_t gpe1_base;
    uint8_t cstate_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;
 
    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t boot_architecture_flags;
 
    uint8_t reserved2;
    uint32_t flags;
 
    // 12 byte structure; see below for details
    generic_address_structure_t reset_reg;
 
    uint8_t reset_value;
    uint8_t reserved3[3];
 
    // 64bit pointers - Available on ACPI 2.0+
    uint64_t x_firmware_control;
    uint64_t x_dsdt;
 
    generic_address_structure_t x_pm1a_event_block;
    generic_address_structure_t x_pm1b_event_block;
    generic_address_structure_t x_pm1a_control_block;
    generic_address_structure_t x_pm1b_control_block;
    generic_address_structure_t x_pm2_control_block;
    generic_address_structure_t x_pm_timer_block;
    generic_address_structure_t x_gpe0_block;
    generic_address_structure_t x_gpe1_block;
} __attribute__ ((packed)) fadt_t;

void init_acpi(void* rsdp);
void* acpi_get_table(char* signature);

extern uint8_t acpi_revision;
extern xsdp_t* acpi_xsdp;

// One of them must be set!
extern rsdt_t* acpi_rsdt;
extern xsdt_t* acpi_xsdt;

extern uint64_t* acpi_tables;
extern uint8_t acpi_table_count;

extern fadt_t* acpi_fadt;