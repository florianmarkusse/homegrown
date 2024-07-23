#ifndef ACPI_MADT_H
#define ACPI_MADT_H

#include "acpi/c-acpi-rsdt.h"
#include "types.h"

typedef struct __attribute((packed)) {
    U8 type;
    U8 totalLength;
    void *data;
} InterruptControllerStructure;

typedef struct __attribute((packed)) {
    CAcpiDescriptionTableHeader header;
    U32 localInterruptControllerAddress;
    U32 flags;
} ConstantMADT;

typedef struct __attribute((packed)) {
    ConstantMADT madt;
    InterruptControllerStructure interruptStructures[];
} MADT;

#endif
