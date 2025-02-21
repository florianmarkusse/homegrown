#ifndef X86_GDT_H
#define X86_GDT_H

#include "shared/types/types.h"
#include "x86/memory/definitions.h"
typedef struct {
    U16 limit;
    U64 base;
} __attribute__((packed)) DescriptorTableRegister;

DescriptorTableRegister *prepNewGDT(PhysicalBasePage zeroPages[3]);
void enableNewGDT(DescriptorTableRegister *GDTRegister);

#endif
