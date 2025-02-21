#ifndef X86_EFI_GDT_H
#define X86_EFI_GDT_H

#include "efi/firmware/base.h"
#include "x86/gdt.h"
extern PhysicalAddress gdtData;
extern DescriptorTableRegister *gdtDescriptor;

#endif
