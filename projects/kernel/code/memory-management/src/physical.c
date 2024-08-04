#include "memory-management/physical.h"
#include "hardware/idt.h"
#include "kernel-parameters.h"
#include "memory/definitions.h"
#include "util/log.h"

typedef struct {
    U64 pageStart;
    U64 numberOfPages;
} FreeMemory;

typedef MAX_LENGTH_ARRAY(FreeMemory) FreeMemory_max_a;

static FreeMemory_max_a freeMemory;

bool canBeUsedByOS(MemoryType type) {
    switch (type) {
    case LOADER_CODE:
    case LOADER_DATA:
    case BOOT_SERVICES_CODE:
    case BOOT_SERVICES_DATA:
    case CONVENTIONAL_MEMORY:
        return true;
    default:
        return false;
    }
}

U64 allocPhysicalPages(U64 numberOfPages) {
    for (U64 i = 0; i < freeMemory.len; i++) {
        if (freeMemory.buf[i].numberOfPages >= numberOfPages) {
            return freeMemory.buf[i].pageStart;
        }
    }

    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);

    __builtin_unreachable();
}

// Coming into this, All the memory is identity mapped.
void initPhysicalMemoryManager(KernelMemory kernelMemory) {
    U64 totalPages = 0;
    U64 freePages = 0;
    for (U64 i = 0; i < kernelMemory.totalDescriptorSize;
         i += kernelMemory.descriptorSize) {
        MemoryDescriptor *descriptor =
            (MemoryDescriptor *)((U8 *)kernelMemory.descriptors + i);

        totalPages += descriptor->number_of_pages;

        if (canBeUsedByOS(descriptor->type)) {
            if (!freeMemory.buf) {
                freeMemory.buf = (FreeMemory *)descriptor->physical_start;
                freeMemory.cap = PAGE_SIZE / sizeof(FreeMemory);
                freeMemory.len = 0;

                descriptor->physical_start += PAGE_SIZE;
                descriptor->number_of_pages--;
                if (descriptor->number_of_pages == 0) {
                    continue;
                }
            }
            if (freeMemory.len >= freeMemory.cap) {
                // Do scmethign
            }

            freeMemory.buf[freeMemory.len] =
                (FreeMemory){.pageStart = descriptor->physical_start,
                             .numberOfPages = descriptor->number_of_pages};
        }
    }

    FLUSH_AFTER {
        LOG(STRING("Total pages:\t"));
        LOG(totalPages, NEWLINE);
        LOG(STRING("Total memory:\t"));
        LOG(totalPages * 4);
        LOG(STRING("Kib"), NEWLINE);
        LOG(STRING("Free pages:\t\t"));
        LOG(freePages, NEWLINE);
        LOG(STRING("Free memory:\t"));
        LOG(freePages * 4);
        LOG(STRING("Kib"));
    }

    FLUSH_AFTER {
        LOG(STRING("Total memory slabs:\t"));
        LOG(freeMemory.len, NEWLINE);
        LOG(STRING("Total memory slabs available:\t"));
        LOG(freeMemory.len, NEWLINE);
        LOG(STRING("Total memory:\t"));
        LOG(totalPages * 4);
        LOG(STRING("Kib"), NEWLINE);
        LOG(STRING("Free pages:\t\t"));
        LOG(freePages, NEWLINE);
        LOG(STRING("Free memory:\t"));
        LOG(freePages * 4);
        LOG(STRING("Kib"));
    }
}
