#include "memory-management/physical.h"
#include "hardware/idt.h"
#include "interoperation/kernel-parameters.h"
#include "interoperation/memory/definitions.h"
#include "util/log.h"

// NOTE This is a shitty PMM. This needs to be rewritten for sure!!!
typedef struct {
    U64 pageStart;
    U64 numberOfPages;
} FreeMemory;

typedef MAX_LENGTH_ARRAY(FreeMemory) FreeMemory_max_a;

static FreeMemory_max_a freeMemory;
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

U64 allocPhysicalPages(U64 numberOfPages) {
    for (U64 i = 0; i < freeMemory.len; i++) {
        if (freeMemory.buf[i].numberOfPages >= numberOfPages) {
            freeMemory.buf[i].numberOfPages -= numberOfPages;
            freeMemory.buf[i].pageStart += numberOfPages * PAGE_SIZE;
            return freeMemory.buf[i].pageStart;
        }
    }

    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);

    __builtin_unreachable();
}

void addPhysicalPages(U64 physicalAddress, U64 numberOfPages) {
    if (freeMemory.len >= freeMemory.cap) {
        FLUSH_AFTER {
            LOG(STRING("Allocated pages ("));
            LOG(pageForPMM);
            LOG(STRING("to Physical Memory "
                       "Manager was too little.\n"));
        }

        FreeMemory *newBuf = (FreeMemory *)allocPhysicalPages(pageForPMM + 1);
        memcpy(newBuf, freeMemory.buf,
               freeMemory.len * sizeof(*freeMemory.buf));
        // The page that just got freed should be added now.
        newBuf[freeMemory.len] = (FreeMemory){.pageStart = (U64)freeMemory.buf,
                                              .numberOfPages = pageForPMM};
        freeMemory.len++;
        freeMemory.buf = newBuf;
        freeMemory.cap = (PAGE_SIZE * (pageForPMM + 1)) / sizeof(FreeMemory);

        pageForPMM++;
    }

    freeMemory.buf[freeMemory.len] = (FreeMemory){
        .pageStart = physicalAddress, .numberOfPages = numberOfPages};
    freeMemory.len++;
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
            // Bootstrapping ourselves here.
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
            addPhysicalPages(descriptor->physical_start,
                             descriptor->number_of_pages);
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
        LOG(STRING("Kib"), NEWLINE);
    }

    FLUSH_AFTER {
        LOG(STRING("Physical Memory status\n\n"));
        LOG(STRING("Total memory slabs:\t"));
        LOG(freeMemory.len, NEWLINE);
        LOG(STRING("First 10 memory slabs in detaul:\n"));
        for (U64 i = 0; i < 10 && i < freeMemory.len; i++) {
            LOG(STRING("Page start:\t"));
            LOG((void *)freeMemory.buf[i].pageStart, NEWLINE);
            LOG(STRING("Number of pages:\t"));
            LOG(freeMemory.buf[i].numberOfPages, NEWLINE);
        }
    }
}
