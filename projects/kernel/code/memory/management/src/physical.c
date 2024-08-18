#include "memory/management/physical.h"
#include "hardware/idt.h" // for triggerFault, FAULT_N...
#include "interoperation/array-types.h"
#include "interoperation/array.h"              // for MAX_LENGTH_ARRAY
#include "interoperation/kernel-parameters.h"  // for KernelMemory
#include "interoperation/memory/definitions.h" // for PAGE_SIZE
#include "interoperation/types.h"              // for U64, U32, U8
#include "memory/manipulation/manipulation.h"
#include "text/string.h" // for STRING
#include "util/assert.h"
#include "util/log.h" // for LOG, LOG_CHOOSER_IMPL_1
#include "util/maths.h"

// NOTE This is a shitty PMM. This needs to be rewritten for sure!!!

static FreeMemory_max_a basePM;
static U32 basePagesForBasePM =
    1; // We bootstrap the base PM with a single memory page, the others use the
       // base PM then to build up their own Page Manager.
static FreeMemory_max_a largePM;
static U32 basePagesForLargePM = 0;
static FreeMemory_max_a hugePM;

#define ENTRIES_IN_BASE_PAGES(basePages)                                       \
    ((PAGE_SIZE * (basePages)) / sizeof(FreeMemory))

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

U64 testTest() { return 123; }

// NOTE: We can add an index on top of the pages if this function becomes an
// issue that is based on available pages.
U64 allocContiguousBasePhysicalPages(U64 numberOfPages) {
    for (U32 i = 0; i < basePM.len; i++) {
        if (basePM.buf[i].numberOfPages >= numberOfPages) {
            basePM.buf[i].numberOfPages -= numberOfPages;
            basePM.buf[i].pageStart += numberOfPages * PAGE_SIZE;
            return basePM.buf[i].pageStart;
        }
    }

    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);

    __builtin_unreachable();
}

FreeMemory_a allocBasePhysicalPages(FreeMemory_a pages) {
    U32 requestedPages = (U32)pages.len;
    U32 contiguousMemoryRegions = 0;

    for (U32 i = 0; i < basePM.len; i++) {
        if (basePM.buf[i].numberOfPages >= requestedPages) {
            pages.buf[contiguousMemoryRegions].numberOfPages = requestedPages;
            pages.buf[contiguousMemoryRegions].pageStart =
                basePM.buf[i].pageStart;

            basePM.buf[i].numberOfPages -= requestedPages;
            basePM.buf[i].pageStart += requestedPages * PAGE_SIZE;

            return pages;
        }

        pages.buf[contiguousMemoryRegions] = basePM.buf[i];
        contiguousMemoryRegions++;
        requestedPages -= basePM.buf[i].numberOfPages;

        basePM.buf[i].numberOfPages = 0;
    }

    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);

    __builtin_unreachable();
}

typedef enum { BASE_PAGE, LARGE_PAGE, HUGE_PAGE } PageType;
#define freePages(pageType)                                                    \
    ((pageType) == PAGE_TYPE_1                                                 \
         ? freePages1()                                                        \
         : ((pageType) == PAGE_TYPE_2 ? freePages2() : freePagesDefault()))

void freeLargePhysicalPages(FreeMemory_a pages) {
    for (U64 i = 0; i < pages.len; i++) {
        if (largePM.len >= largePM.cap) {
            FLUSH_AFTER {
                LOG(STRING("Allocated pages ("));
                LOG(basePagesForLargePM);
                LOG(STRING(") to Large PM "
                           "was too little.\nReallocating...\n"));
            }

            FreeMemory *newBuf = (FreeMemory *)allocContiguousBasePhysicalPages(
                basePagesForLargePM + 1);
            memcpy(newBuf, largePM.buf, largePM.len * sizeof(*largePM.buf));
            if (basePagesForLargePM > 0) {
                // The page that just got freed should be added now.
                newBuf[largePM.len] =
                    (FreeMemory){.pageStart = (U64)largePM.buf,
                                 .numberOfPages = basePagesForLargePM};
                largePM.len++;
            }
            largePM.buf = newBuf;
            largePM.cap = ENTRIES_IN_BASE_PAGES(basePagesForLargePM + 1);

            basePagesForLargePM++;
        }
        largePM.buf[largePM.len] = pages.buf[i];
        largePM.len++;
    }
}

void freeLargePhysicalPage(FreeMemory page) {
    return freeLargePhysicalPages(
        (FreeMemory_a){.len = 1, .buf = (FreeMemory[]){page}});
}

void freeBasePhysicalPages(FreeMemory_a pages) {
    for (U64 i = 0; i < pages.len; i++) {
        if (basePM.len >= basePM.cap) {
            FLUSH_AFTER {
                LOG(STRING("Allocated pages ("));
                LOG(basePagesForBasePM);
                LOG(STRING(") to Base PM "
                           "was too little.\nReallocating...\n"));
            }

            FreeMemory *newBuf = (FreeMemory *)allocContiguousBasePhysicalPages(
                basePagesForBasePM + 1);
            memcpy(newBuf, basePM.buf, basePM.len * sizeof(*basePM.buf));
            // The page that just got freed should be added now.
            newBuf[basePM.len] =
                (FreeMemory){.pageStart = (U64)basePM.buf,
                             .numberOfPages = basePagesForBasePM};
            basePM.len++;
            basePM.buf = newBuf;
            basePM.cap = ENTRIES_IN_BASE_PAGES(basePagesForBasePM + 1);

            basePagesForBasePM++;
        }
        basePM.buf[basePM.len] = pages.buf[i];
        basePM.len++;
    }
}

void freeBasePhysicalPage(FreeMemory page) {
    return freeBasePhysicalPages(
        (FreeMemory_a){.len = 1, .buf = (FreeMemory[]){page}});
}

void appendPMMStatus(FreeMemory_max_a pmm) {
    LOG(STRING("Physical Memory status\n"));
    LOG(STRING("Total free pages:\t"));
    U64 totalPages = 0;
    for (U64 i = 0; i < pmm.len; i++) {
        totalPages += pmm.buf[i].numberOfPages;
    }
    LOG(totalPages, NEWLINE);
    LOG(STRING("Total memory regions:\t"));
    LOG(pmm.len, NEWLINE);
    LOG(STRING("First memory regions in detail:\n"));
    for (U64 i = 0; i < 1 && i < pmm.len; i++) {
        LOG(STRING("Page start:\t"));
        LOG((void *)pmm.buf[i].pageStart, NEWLINE);
        LOG(STRING("Number of pages:\t"));
        LOG(pmm.buf[i].numberOfPages, NEWLINE);
    }
}

void printPhysicalMemoryManagerStatus() {
    FLUSH_AFTER {
        LOG(STRING("Physical Memory status\n"));
        LOG(STRING("================\n"));
        LOG(STRING("Base PM:\n"));
        appendPMMStatus(basePM);
        LOG(STRING("Large PM:\n"));
        appendPMMStatus(largePM);
    }
}

void initLargePM() {
    ASSERT(!basePM.buf);

    for (U64 i = 0; i < basePM.len; i++) {
        FreeMemory memory = basePM.buf[i];

        if (memory.numberOfPages >= PAGE_ENTRY_SIZE) {
            U64 applicablePageBoundary =
                ALIGN_UP(memory.pageStart, PAGE_ENTRY_SHIFT);
            U64 pagesMoved =
                (applicablePageBoundary - memory.pageStart) / PAGE_SIZE;
            U64 pagesFromAlign = memory.numberOfPages - pagesMoved;
            // Now we are actually able to move a value into the large PM.
            if (pagesFromAlign >= PAGE_ENTRY_SIZE) {
                basePM.buf[i].numberOfPages = pagesMoved;
                U64 alignedBasePages =
                    ALIGN_DOWN(pagesFromAlign, PAGE_ENTRY_SHIFT);
                freeLargePhysicalPage((FreeMemory){
                    .pageStart = applicablePageBoundary,
                    .numberOfPages = alignedBasePages >> PAGE_ENTRY_SHIFT});
                U64 leftoverPages = pagesFromAlign - alignedBasePages;
                if (leftoverPages > 0) {
                    freeBasePhysicalPage((FreeMemory){
                        .pageStart = applicablePageBoundary +
                                     (alignedBasePages * PAGE_SIZE),
                        .numberOfPages = leftoverPages});
                }
            }
        }
    }
}

// Coming into this, All the memory is identity mapped.
void initPhysicalMemoryManager(KernelMemory kernelMemory) {
    U64 j = 0;
    MemoryDescriptor *descriptor =
        (MemoryDescriptor *)((U8 *)kernelMemory.descriptors + j);
    while (!canBeUsedByOS(descriptor->type)) {
        j += kernelMemory.descriptorSize;
        descriptor = (MemoryDescriptor *)((U8 *)kernelMemory.descriptors + j);
    }
    basePM.buf = (FreeMemory *)descriptor->physicalStart;
    basePM.cap = ENTRIES_IN_BASE_PAGES(1);
    basePM.len = 0;
    descriptor->physicalStart += PAGE_SIZE;
    descriptor->numberOfPages--;
    if (descriptor->numberOfPages == 0) {
        j += kernelMemory.descriptorSize;
        descriptor = (MemoryDescriptor *)((U8 *)kernelMemory.descriptors + j);
        while (!canBeUsedByOS(descriptor->type)) {
            j += kernelMemory.descriptorSize;
            descriptor =
                (MemoryDescriptor *)((U8 *)kernelMemory.descriptors + j);
        }
    }

    U64 maxCapacity = ENTRIES_IN_BASE_PAGES(descriptor->numberOfPages);
    FreeMemory_a freeMemoryArray = (FreeMemory_a){
        .buf = (FreeMemory *)descriptor->physicalStart, .len = 0};
    // The memory used to store the free memory is also "free" memory
    // after the PMM is correctly initialized.
    FreeMemory freeMemoryHolder =
        (FreeMemory){.pageStart = descriptor->physicalStart,
                     .numberOfPages = descriptor->numberOfPages};
    j += kernelMemory.descriptorSize;

    for (; j < kernelMemory.totalDescriptorSize;
         j += kernelMemory.descriptorSize) {
        descriptor = (MemoryDescriptor *)((U8 *)kernelMemory.descriptors + j);

        if (canBeUsedByOS(descriptor->type)) {
            if (freeMemoryArray.len >= maxCapacity) {
                freeBasePhysicalPages(freeMemoryArray);
                freeMemoryArray.len = 0;
            }
            freeMemoryArray.buf[freeMemoryArray.len] =
                (FreeMemory){.pageStart = descriptor->physicalStart,
                             .numberOfPages = descriptor->numberOfPages};
            freeMemoryArray.len++;
        }
    }
    freeBasePhysicalPages(freeMemoryArray);
    freeBasePhysicalPage(freeMemoryHolder);

    initLargePM();

    printPhysicalMemoryManagerStatus();
}
