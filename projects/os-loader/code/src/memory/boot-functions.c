#include "os-loader/memory/boot-functions.h"

#include "efi/error.h"
#include "efi/firmware/simple-text-output.h"
#include "efi/firmware/system.h"
#include "efi/globals.h"
#include "efi/memory.h"
#include "platform-abstraction/log.h"
#include "platform-abstraction/memory/manipulation.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "x86/memory/definitions.h"

PhysicalAddress allocAndZero(USize numPages) {
    PhysicalAddress page = 0;
    Status status = globals.st->boot_services->allocate_pages(
        ALLOCATE_ANY_PAGES, LOADER_DATA, numPages, &page);
    if (EFI_ERROR(status)) {
        KFLUSH_AFTER { ERROR(STRING("unable to allocate pages!\n")); }
        waitKeyThenReset();
    }

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    memset((void *)page, 0, numPages * UEFI_PAGE_SIZE);
    return page;
}

void mapMemoryAtWithFlags(U64 phys, U64 virt, U64 size, U64 additionalFlags) {
    /* is this a canonical address? We handle virtual memory up to 256TB */
    if (!globals.rootPageTable ||
        ((virt >> 48L) != 0x0000 && (virt >> 48L) != 0xffff)) {
        KFLUSH_AFTER {
            ERROR(STRING(
                "Incorrect address mapped or no page table set up yet!\n"));
        }
        waitKeyThenReset();
    }

    U64 end = virt + size;
    U64 *pageEntry = nullptr;
    /* walk the page tables and add the missing pieces */
    for (virt &= ~(PAGE_MASK), phys &= ~(PAGE_MASK); virt < end;
         virt += PAGE_FRAME_SIZE, phys += PAGE_FRAME_SIZE) {
        /* 512G */
        pageEntry =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            &(((PhysicalAddress *)globals.rootPageTable)[RING_RANGE_VALUE(
                virt >> 39L, PageTableFormat.ENTRIES)]);
        if (!*pageEntry) {
            PhysicalAddress addr = allocAndZero(1);
            *pageEntry = (addr | (VirtualPageMasks.PAGE_PRESENT |
                                  VirtualPageMasks.PAGE_WRITABLE));

            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            KFLUSH_AFTER { INFO((void *)(*pageEntry), NEWLINE); }
        }

        /* 1G */
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        pageEntry = (PhysicalAddress *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(
            pageEntry[RING_RANGE_VALUE(virt >> 30L, PageTableFormat.ENTRIES)]);
        if (!*pageEntry) {
            *pageEntry = (allocAndZero(1) | (VirtualPageMasks.PAGE_PRESENT |
                                             VirtualPageMasks.PAGE_WRITABLE));

            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            KFLUSH_AFTER { INFO((void *)(*pageEntry), NEWLINE); }
        }
        /* 2M  */
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        pageEntry = (PhysicalAddress *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(
            pageEntry[RING_RANGE_VALUE(virt >> 21L, PageTableFormat.ENTRIES)]);
        if (!*pageEntry) {
            *pageEntry = (allocAndZero(1) | (VirtualPageMasks.PAGE_PRESENT |
                                             VirtualPageMasks.PAGE_WRITABLE));
        }
        /* 4K */
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        pageEntry = (PhysicalAddress *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(
            pageEntry[RING_RANGE_VALUE(virt >> 12L, PageTableFormat.ENTRIES)]);
        /* if this page is already mapped, that means the kernel has invalid,
         * overlapping segments */
        if (!*pageEntry) {
            *pageEntry = phys |
                         (VirtualPageMasks.PAGE_PRESENT |
                          VirtualPageMasks.PAGE_WRITABLE) |
                         additionalFlags;
        }
    }
}

void mapMemoryAt(U64 phys, U64 virt, U64 size) {
    mapMemoryAtWithFlags(phys, virt, size, 0);
}

MemoryInfo getMemoryInfo() {
    MemoryInfo mmap = {0};

    // Call GetMemoryMap with initial buffer size of 0 to retrieve the
    // required buffer size
    Status status = globals.st->boot_services->get_memory_map(
        &mmap.memoryMapSize, mmap.memoryMap, &mmap.mapKey, &mmap.descriptorSize,
        &mmap.descriptorVersion);

    if (status != BUFFER_TOO_SMALL) {
        KFLUSH_AFTER {
            ERROR(STRING(
                "Should have received a buffer too small error here!\n"));
        }
        waitKeyThenReset();
    }

    // Some extra because allocating can create extra descriptors and
    // otherwise
    // exitbootservices will fail (lol)
    mmap.memoryMapSize += mmap.descriptorSize * 2;
    status = globals.st->boot_services->allocate_pages(
        ALLOCATE_ANY_PAGES, LOADER_DATA,
        CEILING_DIV_VALUE(mmap.memoryMapSize, UEFI_PAGE_SIZE),
        (PhysicalAddress *)&mmap.memoryMap);
    if (EFI_ERROR(status)) {
        KFLUSH_AFTER {
            ERROR(STRING("Could not allocate data for memory map buffer\n"));
        }
        waitKeyThenReset();
    }

    status = globals.st->boot_services->get_memory_map(
        &mmap.memoryMapSize, mmap.memoryMap, &mmap.mapKey, &mmap.descriptorSize,
        &mmap.descriptorVersion);
    if (EFI_ERROR(status)) {
        KFLUSH_AFTER { ERROR(STRING("Getting memory map failed!\n")); }
        waitKeyThenReset();
    }

    return mmap;
}
