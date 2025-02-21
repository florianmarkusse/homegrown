#ifndef ABSTRACTION_EFI_H
#define ABSTRACTION_EFI_H

#include "shared/types/types.h"
void initArchitecture();
void jumpIntoKernel(U64 stackPointer);

#endif
