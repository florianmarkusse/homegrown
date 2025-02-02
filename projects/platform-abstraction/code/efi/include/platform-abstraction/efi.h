#ifndef PLATFORM_ABSTRACTION_EFI_H
#define PLATFORM_ABSTRACTION_EFI_H

#include "shared/types/types.h"
void initArchitecture();
void jumpIntoKernel(U64 stackPointer);

#endif
