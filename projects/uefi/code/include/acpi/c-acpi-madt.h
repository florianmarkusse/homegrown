#ifndef ACPI_C_ACPI_MADT_H
#define ACPI_C_ACPI_MADT_H

#include "acpi/c-acpi-rsdt.h"

typedef struct __attribute((packed)) {
    CAcpiSDT header;
    CEfiU32 local_controller_addr;
    CEfiU32 flags;
    char madt_entries_begin[];
} CAcpiMADT;

typedef struct __attribute__((packed)) {
    CEfiU8 type;
    CEfiU8 length;
} CAcpiMADTHeader;

typedef struct __attribute__((packed)) {
    CAcpiMADTHeader header;
    CEfiU8 acpi_processor_uid;
    CEfiU8 lapic_id;
    CEfiU32 flags;
} CAcpiMADTLAPIC;

typedef struct __attribute__((packed)) {
    CAcpiMADTHeader header;
    CEfiU8 reserved[2];
    CEfiU32 x2apic_id;
    CEfiU32 flags;
    CEfiU32 acpi_processor_uid;
} CAcpiMADTX2APIC;

typedef struct __attribute__((packed)) {
    CEfiU8 type;
    CEfiU8 length;
    CEfiU8 apic_id;
    CEfiU8 reserved;
    CEfiU32 address;
    CEfiU32 gsib;
} CAcpiMADTIOAPIC;

typedef struct __attribute__((packed)) {
    CAcpiMADTHeader header;
    CEfiU8 reserved1[2];
    CEfiU32 iface_no;
    CEfiU32 acpi_uid;
    CEfiU32 flags;
    CEfiU32 parking_ver;
    CEfiU32 perf_gsiv;
    CEfiU64 parking_addr;
    CEfiU64 gicc_base_addr;
    CEfiU64 gicv_base_addr;
    CEfiU64 gich_base_addr;
    CEfiU32 vgic_maint_gsiv;
    CEfiU64 gicr_base_addr;
    CEfiU64 mpidr;
    CEfiU8 power_eff_class;
    CEfiU8 reserved2;
    CEfiU16 spe_overflow_gsiv;
} CAcpiMADTGICC;

#endif
