#ifndef ABSTRACTION_MEMORY_MANAGEMENT_POLICY_H
#define ABSTRACTION_MEMORY_MANAGEMENT_POLICY_H

#include "shared/types/types.h"
void *allocAndMapExplicit(U64 numberOfPages, U64 preferredPageSizePowerOfTwo);
void *allocAndMap(U64 bytes);
void *allocContiguousAndMap(U64 numberOfPages, U64 preferredPageSizePowerOfTwo);

void freeMapped(U64 start, U64 bytes);

#endif
