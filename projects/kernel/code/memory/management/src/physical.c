#include "memory/management/physical.h"
#include "cpu/idt.h"                           // for triggerFault, FAULT_N...
#include "interoperation/kernel-parameters.h"  // for KernelMemory
#include "interoperation/memory/definitions.h" // for PAGE_FRAME_SIZE
#include "interoperation/memory/descriptor.h"
#include "interoperation/types.h" // for U64, U32, U8
#include "memory/manipulation/manipulation.h"
#include "shared/assert.h"
#include "platform-abstraction/log.h" // for U64, U32, U8
#include "shared/maths/maths.h"

PhysicalMemoryManager basePMM;
PhysicalMemoryManager largePMM;
PhysicalMemoryManager hugePMM;

static U64 toLargerPages(U64 numberOfPages) {
    return numberOfPages >> PAGE_TABLE_SHIFT;
}

static U64 toSmallerPages(U64 numberOfPages) {
    return numberOfPages << PAGE_TABLE_SHIFT;
}

static PageSize toLargerPageSize(PageSize pageSize) {
    return pageSize << PAGE_TABLE_SHIFT;
}

static PageSize toSmallerPageSize(PageSize pageSize) {
    return pageSize >> PAGE_TABLE_SHIFT;
}

static void decreasePages(PhysicalMemoryManager *manager, U64 index,
                          U64 decreaseBy) {
    ASSERT(manager->memory.buf[index].numberOfPages >= decreaseBy);
    if (manager->memory.buf[index].numberOfPages == decreaseBy) {
        manager->memory.buf[index] =
            manager->memory.buf[manager->memory.len - 1];
        manager->memory.len--;
    } else {
        manager->memory.buf[index].numberOfPages -= decreaseBy;
        manager->memory.buf[index].pageStart += decreaseBy * manager->pageSize;
    }
}

static PhysicalMemoryManager *getMemoryManager(PageSize pageSize) {
    switch (pageSize) {
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
static bool canBeUsedByOS(MemoryType type) {
    switch (type) {
    case LOADER_CODE:
    case LOADER_DATA:
    case BOOT_SERVICES_CODE:
    case BOOT_SERVICES_DATA:
    case CONVENTIONAL_MEMORY:
    case PERSISTENT_MEMORY:
        return true;
    default:
        return false;
    }
}

// NOTE: We can add an index on top of the pages if this function becomes an
// issue that is based on available pages.
static U64
allocContiguousPhysicalPagesWithManager(U64 numberOfPages,
                                        PhysicalMemoryManager *manager) {
    for (U32 i = 0; i < manager->memory.len; i++) {
        if (manager->memory.buf[i].numberOfPages >= numberOfPages) {
            U64 address = manager->memory.buf[i].pageStart;
            decreasePages(manager, i, numberOfPages);
            return address;
        }
    }

    if (manager->pageSize < HUGE_PAGE) {
        U64 pagesForLargerManager =
            CEILING_DIV_EXP(numberOfPages, PAGE_TABLE_SHIFT);
        U64 address = allocContiguousPhysicalPages(
            pagesForLargerManager, toLargerPageSize(manager->pageSize));

        U64 freeAddressStart = address + numberOfPages * manager->pageSize;
        U64 freePages = toSmallerPages(pagesForLargerManager) - numberOfPages;

        freePhysicalPage((PagedMemory){.numberOfPages = freePages,
                                       .pageStart = freeAddressStart},
                         manager->pageSize);

        return address;
    }

    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);
}

U64 allocContiguousPhysicalPages(U64 numberOfPages, PageSize pageSize) {
    return allocContiguousPhysicalPagesWithManager(numberOfPages,
                                                   getMemoryManager(pageSize));
}

static PagedMemory_a
allocPhysicalPagesWithManager(PagedMemory_a pages,
                              PhysicalMemoryManager *manager) {
    U32 requestedPages = (U32)pages.len;

    // The final output array may have fewer entries since the memory might be
    // in contiguous blocks.
    pages.len = 0;
    for (U64 i = manager->memory.len - 1; manager->memory.len > 0;
         manager->memory.len--, i = manager->memory.len - 1) {
        if (manager->memory.buf[i].numberOfPages >= requestedPages) {
            pages.buf[pages.len].numberOfPages = requestedPages;
            pages.buf[pages.len].pageStart = manager->memory.buf[i].pageStart;
            pages.len++;

            decreasePages(manager, i, requestedPages);
            return pages;
        }

        pages.buf[pages.len] = manager->memory.buf[i];
        pages.len++;
        requestedPages -= manager->memory.buf[i].numberOfPages;
    }

    if (manager->pageSize < HUGE_PAGE) {
        // Convert to larger manager request
        // Can slyly use the leftover buffer as it is assumed that it is at
        // least big enough for the smaller buffer so definitely true for the
        // larger buffer
        PagedMemory_a leftOverRequest = (PagedMemory_a){
            .buf = pages.buf + pages.len,
            .len = CEILING_DIV_EXP(requestedPages, PAGE_TABLE_SHIFT)};

        PagedMemory_a largerPage = allocPhysicalPages(
            leftOverRequest, toLargerPageSize(manager->pageSize));
        // Convert back to original manager sizes
        for (U64 i = 0; i < largerPage.len; i++) {
            largerPage.buf[i].numberOfPages *= PAGE_TABLE_ENTRIES;
        }
        freePhysicalPages(largerPage, manager->pageSize);

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

PagedMemory_a allocPhysicalPages(PagedMemory_a pages, PageSize pageSize) {
    return allocPhysicalPagesWithManager(pages, getMemoryManager(pageSize));
}

static void freePhysicalPagesWithManager(PagedMemory_a pages,
                                         PhysicalMemoryManager *manager) {
    for (U64 i = 0; i < pages.len; i++) {
        if (manager->memory.len >= manager->memory.cap) {
            PagedMemory *newBuf =
                (PagedMemory *)allocContiguousPhysicalPagesWithManager(
                    toLargerPageSize(manager->usedBasePages), &basePMM);
            memcpy(newBuf, manager->memory.buf,
                   manager->memory.len * sizeof(*manager->memory.buf));
            // The page that just got freed should be added now.
            newBuf[manager->memory.len] =
                (PagedMemory){.pageStart = (U64)manager->memory.buf,
                              .numberOfPages = manager->usedBasePages};
            manager->memory.len++;
            manager->memory.buf = newBuf;
            manager->memory.cap = MEMORY_ENTRIES_IN_BASE_PAGES(
                toLargerPageSize(manager->usedBasePages));

            (manager->usedBasePages)++;
        }
        manager->memory.buf[manager->memory.len] = pages.buf[i];
        manager->memory.len++;
    }
}

void freePhysicalPages(PagedMemory_a page, PageSize pageSize) {
    return freePhysicalPagesWithManager(page, getMemoryManager(pageSize));
}

void freePhysicalPage(PagedMemory page, PageSize pageSize) {
    return freePhysicalPagesWithManager(
        (PagedMemory_a){.len = 1, .buf = (PagedMemory[]){page}},
        getMemoryManager(pageSize));
}

static void initPMM(PageSize pageType) {
    ASSERT(pageType == LARGE_PAGE || pageType == HUGE_PAGE);

    PhysicalMemoryManager *initingManager = getMemoryManager(pageType);
    ASSERT(initingManager->usedBasePages == 1 &&
           initingManager->memory.len == 0);

    initingManager->memory.buf =
        (PagedMemory *)allocContiguousPhysicalPagesWithManager(
            initingManager->usedBasePages, &basePMM);
    initingManager->memory.cap =
        MEMORY_ENTRIES_IN_BASE_PAGES(initingManager->usedBasePages);

    //  12 For the aligned page frame always
    //  9 for every level of page table (large and huge currently)
    U64 initedManagerPageSize = toSmallerPageSize(pageType);
    PhysicalMemoryManager *initedManager =
        getMemoryManager(initedManagerPageSize);
    U64 initingManagerPageSize = pageType;

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
                                  .numberOfPages = (toLargerPages(
                                      alignedForNextLevelPages))},
                    pageType);
                U64 leftoverPages = pagesFromAlign - alignedForNextLevelPages;
                if (leftoverPages > 0) {
                    freePhysicalPage(
                        (PagedMemory){.pageStart = applicablePageBoundary +
                                                   (alignedForNextLevelPages *
                                                    initedManagerPageSize),
                                      .numberOfPages = leftoverPages},
                        initedManagerPageSize);
                }
            }
        }
    }
}

static MemoryDescriptor *nextValidDescriptor(U64 *i,
                                             KernelMemory kernelMemory) {
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
        (PhysicalMemoryManager){.pageSize = BASE_PAGE, .usedBasePages = 1};
    largePMM =
        (PhysicalMemoryManager){.pageSize = LARGE_PAGE, .usedBasePages = 1};
    hugePMM =
        (PhysicalMemoryManager){.pageSize = HUGE_PAGE, .usedBasePages = 1};

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
