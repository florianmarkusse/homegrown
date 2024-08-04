#include "memory-management/physical.h"
#include "hardware/idt.h"                      // for triggerFault, FAULT_N...
#include "interoperation/kernel-parameters.h"  // for KernelMemory
#include "interoperation/memory/definitions.h" // for PAGE_SIZE
#include "interoperation/types.h"              // for U64, U32, U8
#include "util/array.h"                        // for MAX_LENGTH_ARRAY
#include "util/log.h"                          // for LOG, LOG_CHOOSER_IMPL_1
#include "util/memory/memory.h"                // for memcpy
#include "util/text/string.h"                  // for STRING

// NOTE This is a shitty PMM. This needs to be rewritten for sure!!!
typedef struct {
    U64 pageStart;
    U64 numberOfPages;
} FreeMemory;

typedef MAX_LENGTH_ARRAY(FreeMemory) FreeMemory_max_a;

static FreeMemory_max_a PMM;
static U32 pageForPMM = 1;

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

void *allocPhysicalPages(U64 numberOfPages) {
    for (U64 i = 0; i < PMM.len; i++) {
        if (PMM.buf[i].numberOfPages >= numberOfPages) {
            PMM.buf[i].numberOfPages -= numberOfPages;
            PMM.buf[i].pageStart += numberOfPages * PAGE_SIZE;
            return (void *)PMM.buf[i].pageStart;
        }
    }

    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);

    __builtin_unreachable();
}

void addPhysicalPages(U64 physicalAddress, U64 numberOfPages) {
    if (PMM.len >= PMM.cap) {
        FLUSH_AFTER {
            LOG(STRING("Allocated pages ("));
            LOG(pageForPMM);
            LOG(STRING("to Physical Memory "
                       "Manager was too little.\n"));
        }

        FreeMemory *newBuf = (FreeMemory *)allocPhysicalPages(pageForPMM + 1);
        memcpy(newBuf, PMM.buf, PMM.len * sizeof(*PMM.buf));
        // The page that just got freed should be added now.
        newBuf[PMM.len] = (FreeMemory){.pageStart = (U64)PMM.buf,
                                       .numberOfPages = pageForPMM};
        PMM.len++;
        PMM.buf = newBuf;
        PMM.cap = (PAGE_SIZE * (pageForPMM + 1)) / sizeof(FreeMemory);

        pageForPMM++;
    }

    PMM.buf[PMM.len] = (FreeMemory){.pageStart = physicalAddress,
                                    .numberOfPages = numberOfPages};
    PMM.len++;
}

void printPhysicalMemoryManagerStatus() {
    U64 totalPages = 0;
    for (U64 i = 0; i < PMM.len; i++) {
        totalPages += PMM.buf[i].numberOfPages;
    }

    FLUSH_AFTER {
        LOG(STRING("Physical Memory status\n"));
        LOG(STRING("Total free pages:\t"));
        LOG(totalPages, NEWLINE);
        LOG(STRING("Total memory slabs:\t"));
        LOG(PMM.len, NEWLINE);
        LOG(STRING("First 3 memory slabs in detail:\n"));
        for (U64 i = 0; i < 3 && i < PMM.len; i++) {
            LOG(STRING("Page start:\t"));
            LOG((void *)PMM.buf[i].pageStart, NEWLINE);
            LOG(STRING("Number of pages:\t"));
            LOG(PMM.buf[i].numberOfPages, NEWLINE);
        }
    }
}

// Coming into this, All the memory is identity mapped.
void initPhysicalMemoryManager(KernelMemory kernelMemory) {
    for (U64 i = 0; i < kernelMemory.totalDescriptorSize;
         i += kernelMemory.descriptorSize) {
        MemoryDescriptor *descriptor =
            (MemoryDescriptor *)((U8 *)kernelMemory.descriptors + i);

        if (canBeUsedByOS(descriptor->type)) {
            // Bootstrapping ourselves here.
            if (!PMM.buf) {
                PMM.buf = (FreeMemory *)descriptor->physical_start;
                PMM.cap = PAGE_SIZE / sizeof(FreeMemory);
                PMM.len = 0;

                descriptor->physical_start += PAGE_SIZE;
                descriptor->number_of_pages--;
                if (descriptor->number_of_pages == 0) {
                    continue;
                }
            }
            addPhysicalPages(descriptor->physical_start,
                             descriptor->number_of_pages);
        }
    }

    printPhysicalMemoryManagerStatus();
}
