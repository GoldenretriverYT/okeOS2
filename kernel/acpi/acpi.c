#include "acpi.h"

xsdp_t* acpi_xsdp;
uint8_t acpi_revision;

rsdt_t* acpi_rsdt;
xsdt_t* acpi_xsdt;

uint64_t* acpi_tables;
uint8_t acpi_table_count;

fadt_t* acpi_fadt;

void init_acpi(void* rsdp) {
    acpi_tables = (uint64_t*)PHYS2HIHA(pmm_alloc_purpose(1, "ACPI"));

    void* xsdp_ptr = (rsdp);
    kprintf("ACPI RSDP/XSDP at %llx\n", xsdp_ptr);

    acpi_xsdp = (xsdp_t*)(xsdp_ptr);
    acpi_revision = acpi_xsdp->revision;

    kprintf("Found ACPI revision %d (version %s)\n", acpi_revision, acpi_revision == 0 ? "1.0" : "2.0+");

    sdt_header_t* acpi_rsdt_header;

    if (acpi_revision == 0) {
        acpi_rsdt_header = (sdt_header_t*)(PHYS2HIHA(acpi_xsdp->rsdt_address));
    } else {
        acpi_rsdt_header = (sdt_header_t*)(PHYS2HIHA(acpi_xsdp->xsdt_address));
    }

    kprintf("ACPI RSDT/XSDT at %llx\n", acpi_rsdt_header);

    ON_ACPI1 {
        rsdt_t* rsdt = (rsdt_t*)(acpi_rsdt_header);
        acpi_rsdt = rsdt;

        kprintf("ACPI RSDT at %llx\n", rsdt);

        uint64_t entry_count = (rsdt->header.length - sizeof(sdt_header_t)) / 4;
        kprintf("ACPI RSDT has %llu entries\n", entry_count);

        for (uint64_t i = 0; i < entry_count; i++) {
            uint64_t entry = PHYS2HIHA(rsdt->table_pointers[i]);
            char signature[5];

            memcpy(signature, (char*)entry, 4);
            signature[4] = 0;

            kprintf("ACPI RSDT entry %llu at %llx (%s)\n", i, entry, signature);

            acpi_tables[i] = entry;
            acpi_table_count++;
        }
    }
    
    ON_ACPI2 {
        xsdt_t* xsdt = (xsdt_t*)(acpi_rsdt_header);
        acpi_xsdt = xsdt;

        kprintf("ACPI XSDT at %llx\n", xsdt);

        uint64_t entry_count = (xsdt->header.length - sizeof(sdt_header_t)) / 8;
        kprintf("ACPI XSDT has %llu entries\n", entry_count);

        for (uint64_t i = 0; i < entry_count; i++) {
            uint64_t entry = PHYS2HIHA(xsdt->table_pointers[i]);
            char signature[5];

            memcpy(signature, (char*)entry, 4);
            signature[4] = 0;

            kprintf("ACPI XSDT entry %llu at %llx (%s)\n", i, entry, signature);

            acpi_tables[i] = entry;
            acpi_table_count++;
        }
    }

    kprintf("ACPI has %llu tables\n", acpi_table_count);

    // Find FADT
    acpi_fadt = (fadt_t*)acpi_get_table("FACP");
    if(acpi_fadt == NULL) {
        panic("ACPI: FADT not found");
    }

    kprintf("ACPI FADT at %llx\n", acpi_fadt);

    outb(acpi_fadt->smi_command_port, acpi_fadt->acpi_enable);
    while (inw(acpi_fadt->pm1a_control_block) & 1 == 0);

    kprintf("ACPI enabled\n");
}

void* acpi_get_table(char* signature) {
    for (uint64_t i = 0; i < acpi_table_count; i++) {
        uint64_t entry = acpi_tables[i];
        char entry_signature[5];

        memcpy(entry_signature, (char*)entry, 4);
        entry_signature[4] = 0;

        if (strcmp(entry_signature, signature) == 0) {
            return (void*)entry;
        }
    }

    return NULL;
}