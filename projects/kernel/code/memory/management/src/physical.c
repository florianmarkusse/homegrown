#include "memory/management/physical.h"
#include "cpu/idt.h"                           // for triggerFault, FAULT_N...
#include "interoperation/kernel-parameters.h"  // for KernelMemory
#include "interoperation/memory/definitions.h" // for PAGE_FRAME_SIZE
#include "interoperation/memory/descriptor.h"
#include "interoperation/types.h" // for U64, U32, U8
#include "log/log.h"              // for LOG, LOG_CHOOSER_IMPL_1
#include "memory/manipulation/manipulation.h"
#include "text/string.h" // for STRING
#include "util/assert.h"
#include "util/maths.h"

typedef struct {
    PagedMemory_max_a memory;
    U32 usedBasePages;
    PageType pageType;
} PhysicalMemoryManager;

static PhysicalMemoryManager basePMM;
static PhysicalMemoryManager largePMM;
static PhysicalMemoryManager hugePMM;

void decreasePages(PhysicalMemoryManager *manager, U64 index, U64 decreaseBy) {
    ASSERT(manager->memory.buf[index].numberOfPages >= decreaseBy);
    if (manager->memory.buf[index].numberOfPages == decreaseBy) {
        manager->memory.buf[index] =
            manager->memory.buf[manager->memory.len - 1];
        manager->memory.len--;
    } else {
        manager->memory.buf[index].numberOfPages -= decreaseBy;
        manager->memory.buf[index].pageStart +=
            decreaseBy *
            ((manager->pageType * PAGE_TABLE_SHIFT) + PAGE_FRAME_SIZE);
    }
}

PhysicalMemoryManager *getMemoryManager(PageType pageType) {
    switch (pageType) {
    case BASE_PAGE: {
        return &basePMM;
        break;
    }
    case LARGE_PAGE: {
        return &largePMM;
        break;
    }
    case HUGE_PAGE: {
        return &hugePMM;
        break;
    }
    default: {
        __builtin_unreachable();
    }
    }
}

#define MEMORY_ENTRIES_IN_BASE_PAGES(basePages)                                \
    ((PAGE_FRAME_SIZE * (basePages)) / sizeof(PagedMemory))

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
U64 allocContiguousPhysicalPagesWithManager(U64 numberOfPages,
                                            PhysicalMemoryManager *manager) {
    for (U32 i = 0; i < manager->memory.len; i++) {
        if (manager->memory.buf[i].numberOfPages >= numberOfPages) {
            U64 address = manager->memory.buf[i].pageStart;
            decreasePages(manager, i, numberOfPages);
            return address;
        }
    }

    if (manager->pageType < HUGE_PAGE) {
        U64 pagesForLargerManager =
            CEILING_DIV_EXP(numberOfPages, PAGE_TABLE_SHIFT);
        U64 address = allocContiguousPhysicalPages(pagesForLargerManager,
                                                   manager->pageType + 1);

        U64 pageSize =
            1 << (PAGE_FRAME_SHIFT + manager->pageType * PAGE_TABLE_SHIFT);

        U64 freeAddressStart = address + numberOfPages * pageSize;
        U64 freePages =
            (pagesForLargerManager << PAGE_TABLE_SHIFT) - numberOfPages;

        freePhysicalPage((PagedMemory){.numberOfPages = freePages,
                                       .pageStart = freeAddressStart},
                         manager->pageType);

        return address;
    }

    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);
}

U64 allocContiguousPhysicalPages(U64 numberOfPages, PageType pageType) {
    return allocContiguousPhysicalPagesWithManager(numberOfPages,
                                                   getMemoryManager(pageType));
}

PagedMemory_a allocPhysicalPagesWithManager(PagedMemory_a pages,
                                            PhysicalMemoryManager *manager) {
    U32 requestedPages = (U32)pages.len;
    U32 contiguousMemoryRegions = 0;

    // The final output array may have fewer entries since the memory might be
    // in contiguous blocks.
    pages.len = 0;
    for (U64 i = manager->memory.len - 1; manager->memory.len > 0;
         manager->memory.len--, i = manager->memory.len - 1) {
        pages.len++;
        if (manager->memory.buf[i].numberOfPages >= requestedPages) {
            pages.buf[contiguousMemoryRegions].numberOfPages = requestedPages;
            pages.buf[contiguousMemoryRegions].pageStart =
                manager->memory.buf[i].pageStart;

            decreasePages(manager, i, requestedPages);
            return pages;
        }

        pages.buf[contiguousMemoryRegions] = manager->memory.buf[i];
        contiguousMemoryRegions++;
        requestedPages -= manager->memory.buf[i].numberOfPages;
    }

    if (manager->pageType < HUGE_PAGE) {
        // Convert to larger manager request
        // Can slyly use the leftover buffer as it is assumed that it is at
        // least big enough for the smaller buffer so definitely true for the
        // larger buffer
        PagedMemory_a leftOverRequest = (PagedMemory_a){
            .buf = pages.buf + contiguousMemoryRegions,
            .len = CEILING_DIV_EXP(requestedPages, PAGE_TABLE_SHIFT)};

        PagedMemory_a largerPage =
            allocPhysicalPages(leftOverRequest, manager->pageType + 1);
        // Convert back to original manager sizes
        for (U64 i = 0; i < largerPage.len; i++) {
            largerPage.buf[i].numberOfPages *= PAGE_TABLE_ENTRIES;
        }
        freePhysicalPages(largerPage, manager->pageType);

        // The actual leftover request can now be satisfied
        leftOverRequest.len = requestedPages;
        PagedMemory_a addedPages =
            allocPhysicalPagesWithManager(leftOverRequest, manager);

        // "Concat" both arrays, the original found pages and those available
        // after stealing from big brother
        pages.len += addedPages.len;
        return pages;
    }

    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);
}

PagedMemory_a allocPhysicalPages(PagedMemory_a pages, PageType pageType) {
    return allocPhysicalPagesWithManager(pages, getMemoryManager(pageType));
}

void freePhysicalPagesWithManager(PagedMemory_a pages,
                                  PhysicalMemoryManager *manager) {
    for (U64 i = 0; i < pages.len; i++) {
        if (manager->memory.len >= manager->memory.cap) {
            PagedMemory *newBuf =
                (PagedMemory *)allocContiguousPhysicalPagesWithManager(
                    manager->usedBasePages + 1, &basePMM);
            memcpy(newBuf, manager->memory.buf,
                   manager->memory.len * sizeof(*manager->memory.buf));
            // The page that just got freed should be added now.
            newBuf[manager->memory.len] =
                (PagedMemory){.pageStart = (U64)manager->memory.buf,
                              .numberOfPages = manager->usedBasePages};
            manager->memory.len++;
            manager->memory.buf = newBuf;
            manager->memory.cap =
                MEMORY_ENTRIES_IN_BASE_PAGES(manager->usedBasePages + 1);

            (manager->usedBasePages)++;
        }
        manager->memory.buf[manager->memory.len] = pages.buf[i];
        manager->memory.len++;
    }
}

void freePhysicalPages(PagedMemory_a page, PageType pageType) {
    return freePhysicalPagesWithManager(page, getMemoryManager(pageType));
}

void freePhysicalPage(PagedMemory page, PageType pageType) {
    return freePhysicalPagesWithManager(
        (PagedMemory_a){.len = 1, .buf = (PagedMemory[]){page}},
        getMemoryManager(pageType));
}

void appendPMMStatus(PhysicalMemoryManager manager) {
    LOG(STRING("Type: "));
    LOG(pageTypeToString[manager.pageType], NEWLINE);
    LOG(STRING("Used base page frames for internal structure: "));
    LOG(manager.usedBasePages, NEWLINE);
    LOG(STRING("Free pages:\t"));
    U64 totalPages = 0;
    for (U64 i = 0; i < manager.memory.len; i++) {
        LOG(manager.memory.buf[i].numberOfPages);
        LOG(STRING(" "));
        totalPages += manager.memory.buf[i].numberOfPages;
    }
    LOG(STRING(" Total: "));
    LOG(totalPages, NEWLINE);
    LOG(STRING("Total memory regions:\t"));
    LOG(manager.memory.len, NEWLINE);
}

void printPhysicalMemoryManagerStatus() {
    FLUSH_AFTER {
        LOG(STRING("Physical Memory status\n"));
        LOG(STRING("================\n"));
        appendPMMStatus(basePMM);
        LOG(STRING("================\n"));
        appendPMMStatus(largePMM);
        LOG(STRING("================\n"));
        appendPMMStatus(hugePMM);
        LOG(STRING("================\n"));
    }
}

void initPMM(PageType pageType) {
    ASSERT(pageType == LARGE_PAGE || pageType == HUGE_PAGE);

    PhysicalMemoryManager *initingManager = getMemoryManager(pageType);
    ASSERT(initingManager->usedBasePages == 1 &&
           initingManager->memory.len == 0);

    initingManager->memory.buf =
        (PagedMemory *)allocContiguousPhysicalPagesWithManager(
            initingManager->usedBasePages, &basePMM);
    initingManager->memory.cap =
        MEMORY_ENTRIES_IN_BASE_PAGES(initingManager->usedBasePages);

    PageType decrementedPageType = pageType - 1;
    PhysicalMemoryManager *initedManager =
        getMemoryManager(decrementedPageType);

    //  12 For the aligned page frame always
    //  9 for every level of page table (large and huge currently)
    U64 initedManagerPageSize = pageTypeToPageSize[decrementedPageType];
    U64 initingManagerPageSize = initedManagerPageSize << PAGE_TABLE_SHIFT;

    for (U64 i = 0; i < initedManager->memory.len; i++) {
        PagedMemory memory = initedManager->memory.buf[i];

        if (memory.numberOfPages >= PAGE_TABLE_ENTRIES) {
            U64 applicablePageBoundary =
                ALIGN_UP_VALUE(memory.pageStart, initingManagerPageSize);
            U64 pagesMoved = (applicablePageBoundary - memory.pageStart) /
                             initedManagerPageSize;
            U64 pagesFromAlign = memory.numberOfPages - pagesMoved;
            // Now we are actually able to move pages into the PMM at the higher
            // level.
            if (pagesFromAlign >= PAGE_TABLE_ENTRIES) {
                decreasePages(initedManager, i, pagesFromAlign);
                // We may have 600 free pages that start on the aligned
                // boundary, but the size of the next level of physical memory
                // manager is 512 * current level, so we can only "upgrade"
                // every 512 pages to 1 new page. The rest is leftover and will
                // be added back to the current level.
                U64 alignedForNextLevelPages =
                    ALIGN_DOWN_EXP(pagesFromAlign, PAGE_TABLE_SHIFT);
                freePhysicalPage(
                    (PagedMemory){.pageStart = applicablePageBoundary,
                                  .numberOfPages = alignedForNextLevelPages >>
                                                   PAGE_TABLE_SHIFT},
                    pageType);
                U64 leftoverPages = pagesFromAlign - alignedForNextLevelPages;
                if (leftoverPages > 0) {
                    freePhysicalPage(
                        (PagedMemory){.pageStart = applicablePageBoundary +
                                                   (alignedForNextLevelPages *
                                                    initedManagerPageSize),
                                      .numberOfPages = leftoverPages},
                        decrementedPageType);
                }
            }
        }
    }
}

MemoryDescriptor *nextValidDescriptor(U64 *i, KernelMemory kernelMemory) {
    while (*i < kernelMemory.totalDescriptorSize) {
        MemoryDescriptor *result =
            (MemoryDescriptor *)((U8 *)kernelMemory.descriptors + *i);
        // Always increment even if found, so the next caller wont get the same
        // descriptor.
        *i += kernelMemory.descriptorSize;
        if (canBeUsedByOS(result->type)) {
            return result;
        }
    }

    return NULL;
}

// Coming into this, All the memory is identity mapped.
// Having to do some boostrapping here with the base page frame physical
// manager.
void initPhysicalMemoryManager(KernelMemory kernelMemory) {
    // Reset the PMMs if they were used previously already
    basePMM =
        (PhysicalMemoryManager){.pageType = BASE_PAGE, .usedBasePages = 1};
    largePMM =
        (PhysicalMemoryManager){.pageType = LARGE_PAGE, .usedBasePages = 1};
    hugePMM =
        (PhysicalMemoryManager){.pageType = HUGE_PAGE, .usedBasePages = 1};

    U64 i = 0;

    MemoryDescriptor *descriptor = nextValidDescriptor(&i, kernelMemory);
    if (!descriptor) {
        triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);
    }

    basePMM.memory.buf = (PagedMemory *)descriptor->physicalStart;
    basePMM.memory.cap = MEMORY_ENTRIES_IN_BASE_PAGES(1);
    basePMM.memory.len = 0;
    descriptor->physicalStart += PAGE_FRAME_SIZE;
    descriptor->numberOfPages--;

    if (descriptor->numberOfPages == 0) {
        descriptor = nextValidDescriptor(&i, kernelMemory);
        if (!descriptor) {
            triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);
        }
    }

    U64 maxCapacity = MEMORY_ENTRIES_IN_BASE_PAGES(descriptor->numberOfPages);
    PagedMemory_a freeMemoryArray = (PagedMemory_a){
        .buf = (PagedMemory *)descriptor->physicalStart, .len = 0};
    // The memory used to store the free memory is also "free" memory
    // after the PMM is correctly initialized.
    PagedMemory freeMemoryHolder =
        (PagedMemory){.pageStart = descriptor->physicalStart,
                      .numberOfPages = descriptor->numberOfPages};

    while ((descriptor = nextValidDescriptor(&i, kernelMemory))) {
        if (freeMemoryArray.len >= maxCapacity) {
            freePhysicalPages(freeMemoryArray, BASE_PAGE);
            freeMemoryArray.len = 0;
        }
        freeMemoryArray.buf[freeMemoryArray.len] =
            (PagedMemory){.pageStart = descriptor->physicalStart,
                          .numberOfPages = descriptor->numberOfPages};
        freeMemoryArray.len++;
    }

    freePhysicalPages(freeMemoryArray, BASE_PAGE);
    freePhysicalPage(freeMemoryHolder, BASE_PAGE);

    initPMM(LARGE_PAGE);
    initPMM(HUGE_PAGE);
}
