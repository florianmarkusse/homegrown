#ifndef X86_MEMORY_PHYSICAL_H
#define X86_MEMORY_PHYSICAL_H

#include "interoperation/kernel-parameters.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions/virtual.h"

typedef struct {
    U8 data[PAGE_FRAME_SIZE];
} PhysicalBasePage;

typedef struct {
    PagedMemory_max_a memory;
    U32 usedBasePages;
    PageSize pageSize;
} PhysicalMemoryManager;

extern PhysicalMemoryManager basePMM;
extern PhysicalMemoryManager largePMM;
extern PhysicalMemoryManager hugePMM;

#endif
