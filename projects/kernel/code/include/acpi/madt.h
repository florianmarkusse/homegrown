#ifndef ACPI_MADT_H
#define ACPI_MADT_H

#include "acpi/c-acpi-rsdt.h"
#include "util/types.h"

typedef struct __attribute((packed)) {
    uint8_t type;
    uint8_t totalLength;
    void *data;
} InterruptControllerStructure;

typedef struct __attribute((packed)) {
    CAcpiDescriptionTableHeader header;
    uint32_t localInterruptControllerAddress;
    uint32_t flags;
} ConstantMADT;

typedef struct __attribute((packed)) {
    ConstantMADT madt;
    InterruptControllerStructure interruptStructures[];
} MADT;

#endif
