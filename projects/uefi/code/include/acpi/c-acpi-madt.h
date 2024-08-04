#ifndef ACPI_C_ACPI_MADT_H
#define ACPI_C_ACPI_MADT_H

#include "acpi/c-acpi-rsdt.h"
#include "interoperation/types.h"

typedef struct __attribute((packed)) {
    CAcpiSDT header;
    U32 local_controller_addr;
    U32 flags;
    I8 madt_entries_begin[];
} CAcpiMADT;

typedef struct __attribute__((packed)) {
    U8 type;
    U8 length;
} CAcpiMADTHeader;

typedef struct __attribute__((packed)) {
    CAcpiMADTHeader header;
    U8 acpi_processor_uid;
    U8 lapic_id;
    U32 flags;
} CAcpiMADTLAPIC;

typedef struct __attribute__((packed)) {
    CAcpiMADTHeader header;
    U8 reserved[2];
    U32 x2apic_id;
    U32 flags;
    U32 acpi_processor_uid;
} CAcpiMADTX2APIC;

typedef struct __attribute__((packed)) {
    U8 type;
    U8 length;
    U8 apic_id;
    U8 reserved;
    U32 address;
    U32 gsib;
} CAcpiMADTIOAPIC;

typedef struct __attribute__((packed)) {
    CAcpiMADTHeader header;
    U8 reserved1[2];
    U32 iface_no;
    U32 acpi_uid;
    U32 flags;
    U32 parking_ver;
    U32 perf_gsiv;
    U64 parking_addr;
    U64 gicc_base_addr;
    U64 gicv_base_addr;
    U64 gich_base_addr;
    U32 vgic_maint_gsiv;
    U64 gicr_base_addr;
    U64 mpidr;
    U8 power_eff_class;
    U8 reserved2;
    U16 spe_overflow_gsiv;
} CAcpiMADTGICC;

#endif
