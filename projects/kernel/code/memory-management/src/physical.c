#include "memory-management/physical.h"
#include "hardware/idt.h"                      // for triggerFault, FAULT_N...
#include "interoperation/kernel-parameters.h"  // for KernelMemory
#include "interoperation/memory/definitions.h" // for PAGE_SIZE
#include "interoperation/types.h"              // for U64, U32, U8
#include "util/array-types.h"
#include "util/array.h" // for MAX_LENGTH_ARRAY
#include "util/assert.h"
#include "util/log.h"           // for LOG, LOG_CHOOSER_IMPL_1
#include "util/memory/memory.h" // for memcpy
#include "util/text/string.h"   // for STRING

// NOTE This is a shitty PMM. This needs to be rewritten for sure!!!

static FreeMemory_max_a basePageManager;
static FreeMemory_max_a largePageManager;
static FreeMemory_max_a hugePageManager;
static U32 pageForPMM = 1;

#define ENTRIES_IN_BASE_PAGE (PAGE_SIZE / sizeof(FreeMemory))

// TODO: table 7.10 UEFI spec section 7.2 - 7.2.1 , not fully complete yet I
// think?
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

// NOTE: We can add an index on top of the pages if this function becomes an
// issue that is based on available pages.
U64 allocContiguousBasePhysicalPages(U64 numberOfPages) {
    for (U32 i = 0; i < basePageManager.len; i++) {
        if (basePageManager.buf[i].numberOfPages >= numberOfPages) {
            basePageManager.buf[i].numberOfPages -= numberOfPages;
            basePageManager.buf[i].pageStart += numberOfPages * PAGE_SIZE;
            return basePageManager.buf[i].pageStart;
        }
    }

    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);

    __builtin_unreachable();
}

FreeMemory_a allocBasePhysicalPages(FreeMemory_a pages) {
    U32 requestedPages = (U32)pages.len;
    U32 contiguousMemoryRegions = 0;

    for (U32 i = 0; i < basePageManager.len; i++) {
        if (basePageManager.buf[i].numberOfPages >= requestedPages) {
            pages.buf[contiguousMemoryRegions].numberOfPages = requestedPages;
            pages.buf[contiguousMemoryRegions].pageStart =
                basePageManager.buf[i].pageStart;

            basePageManager.buf[i].numberOfPages -= requestedPages;
            basePageManager.buf[i].pageStart += requestedPages * PAGE_SIZE;

            return pages;
        }

        pages.buf[contiguousMemoryRegions] = basePageManager.buf[i];
        contiguousMemoryRegions++;
        requestedPages -= basePageManager.buf[i].numberOfPages;

        basePageManager.buf[i].numberOfPages = 0;
    }

    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);

    __builtin_unreachable();
}

void freeBasePhysicalPages(FreeMemory_a pages) {
    for (U64 i = 0; i < pages.len; i++) {
        if (basePageManager.len >= basePageManager.cap) {
            FLUSH_AFTER {
                LOG(STRING("Allocated pages ("));
                LOG(pageForPMM);
                LOG(STRING("to Physical Memory "
                           "Manager was too little.\nReallocating..."));
            }

            FreeMemory *newBuf =
                (FreeMemory *)allocContiguousBasePhysicalPages(pageForPMM + 1);
            memcpy(newBuf, basePageManager.buf,
                   basePageManager.len * sizeof(*basePageManager.buf));
            // The page that just got freed should be added now.
            newBuf[basePageManager.len] =
                (FreeMemory){.pageStart = (U64)basePageManager.buf,
                             .numberOfPages = pageForPMM};
            basePageManager.len++;
            basePageManager.buf = newBuf;
            basePageManager.cap =
                (PAGE_SIZE * (pageForPMM + 1)) / sizeof(FreeMemory);

            pageForPMM++;
        }
        basePageManager.buf[basePageManager.len] = pages.buf[i];
        basePageManager.len++;
    }
}

void printPhysicalMemoryManagerStatus() {
    U64 totalPages = 0;
    for (U64 i = 0; i < basePageManager.len; i++) {
        totalPages += basePageManager.buf[i].numberOfPages;
    }

    FLUSH_AFTER {
        LOG(STRING("Physical Memory status\n"));
        LOG(STRING("Total free pages:\t"));
        LOG(totalPages, NEWLINE);
        LOG(STRING("Total memory slabs:\t"));
        LOG(basePageManager.len, NEWLINE);
        LOG(STRING("First 3 memory slabs in detail:\n"));
        for (U64 i = 0; i < 3 && i < basePageManager.len; i++) {
            LOG(STRING("Page start:\t"));
            LOG((void *)basePageManager.buf[i].pageStart, NEWLINE);
            LOG(STRING("Number of pages:\t"));
            LOG(basePageManager.buf[i].numberOfPages, NEWLINE);
        }
    }
}

// Coming into this, All the memory is identity mapped.
void initPhysicalMemoryManager(KernelMemory kernelMemory) {
    FreeMemory basePhysicalPagesBuf[ENTRIES_IN_BASE_PAGE];
    FreeMemory_a basePhysicalPages =
        (FreeMemory_a){.buf = basePhysicalPagesBuf, .len = 0};

    for (U64 i = 0; i < kernelMemory.totalDescriptorSize;
         i += kernelMemory.descriptorSize) {
        MemoryDescriptor *descriptor =
            (MemoryDescriptor *)((U8 *)kernelMemory.descriptors + i);

        if (canBeUsedByOS(descriptor->type)) {
            ASSERT(basePhysicalPages.len < ENTRIES_IN_BASE_PAGE);
            basePhysicalPages.buf[basePhysicalPages.len] =
                (FreeMemory){.pageStart = descriptor->physical_start,
                             .numberOfPages = descriptor->number_of_pages};
            basePhysicalPages.len++;
        }
    }

    // Bootstrapping ourselves here.
    basePageManager.buf = (FreeMemory *)basePhysicalPagesBuf[0].pageStart;
    basePageManager.cap = ENTRIES_IN_BASE_PAGE;
    basePageManager.len = 0;

    basePhysicalPages.buf[0].pageStart += PAGE_SIZE;
    basePhysicalPages.buf[0].numberOfPages--;
    if (basePhysicalPages.buf[0].numberOfPages == 0) {
        basePhysicalPages.buf++;
        basePhysicalPages.len--;
    }

    freeBasePhysicalPages(basePhysicalPages);

    /*for (U64 i = 0; i < kernelMemory.totalDescriptorSize;*/
    /*     i += kernelMemory.descriptorSize) {*/
    /*    MemoryDescriptor *descriptor =*/
    /*        (MemoryDescriptor *)((U8 *)kernelMemory.descriptors + i);*/
    /**/
    /*    if (canBeUsedByOS(descriptor->type)) {*/
    /*        // Bootstrapping ourselves here.*/
    /*        if (!basePageManager.buf) {*/
    /*            basePageManager.buf = (FreeMemory
     * *)descriptor->physical_start;*/
    /*            basePageManager.cap = ENTRIES_IN_BASE_PAGE;*/
    /*            basePageManager.len = 0;*/
    /**/
    /*            descriptor->physical_start += PAGE_SIZE;*/
    /*            descriptor->number_of_pages--;*/
    /*            if (descriptor->number_of_pages == 0) {*/
    /*                continue;*/
    /*            }*/
    /*        }*/
    /*        addBasePhysicalPages(descriptor->physical_start,*/
    /*                             descriptor->number_of_pages);*/
    /*    }*/
    /*}*/

    printPhysicalMemoryManagerStatus();
}
