#ifndef MEMORY_MANAGEMENT_POLICY_H
#define MEMORY_MANAGEMENT_POLICY_H

#include "platform-abstraction/memory/management/virtual.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"

void *allocContiguousAndMap(U64 numberOfPages, PageSize pageSize);
void *allocAndMapExplicit(PagedMemory_a request, PageSize pageSize);
void *allocAndMap(U64 bytes);

void freeMapped(U64 start, U64 bytes);

#endif
