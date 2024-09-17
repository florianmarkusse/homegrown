#ifndef MEMORY_MANAGEMENT_POLICY_H
#define MEMORY_MANAGEMENT_POLICY_H

#include "memory/management/definitions.h"

void *allocContiguousAndMap(U64 numberOfPages, PageSize pageSize);
void *allocAndMap(PagedMemory_a request, PageSize pageSize);

#endif
