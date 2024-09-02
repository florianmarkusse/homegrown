#ifndef MEMORY_MANAGEMENT_VIRTUAL_H
#define MEMORY_MANAGEMENT_VIRTUAL_H

#include "interoperation/types.h"

void mapMemoryAt(U64 phys, U64 virt, U64 size, U64 additionalFlags);

#endif
