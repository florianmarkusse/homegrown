#include "efi/memory/boot-functions.h"

#include "efi/efi/c-efi-protocol-simple-text-output.h"
#include "efi/efi/c-efi-system.h"
#include "efi/globals.h"
#include "efi/memory/page-size.h"
#include "efi/printing.h"
#include "platform-abstraction/memory/manipulation.h"
#include "shared/maths/maths.h"
#include "x86/memory/definitions/virtual.h"

PhysicalAddress allocAndZero(USize numPages) {
    PhysicalAddress page = 0;
    Status status = globals.st->boot_services->allocate_pages(
        ALLOCATE_ANY_PAGES, LOADER_DATA, numPages, &page);
    if (ERROR(status)) {
        error(u"unable to allocate pages!\r\n");
    }

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    memset((void *)page, 0, numPages * PAGE_FRAME_SIZE);
    return page;
}

void mapMemoryAtWithFlags(U64 phys, U64 virt, U64 size, U64 additionalFlags) {
    /* is this a canonical address? We handle virtual memory up to 256TB */
    if (!globals.level4PageTable ||
        ((virt >> 48L) != 0x0000 && (virt >> 48L) != 0xffff)) {
        error(u"Incorrect address mapped or no page table set up yet!\r\n");
    }

    U64 end = virt + size;
    U64 *pageEntry = nullptr;
    /* walk the page tables and add the missing pieces */
    for (virt &= ~(PAGE_MASK), phys &= ~(PAGE_MASK); virt < end;
         virt += PAGE_FRAME_SIZE, phys += PAGE_FRAME_SIZE) {
        /* 512G */
        pageEntry =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            &(((PhysicalAddress *)globals.level4PageTable)[RING_RANGE_VALUE(
                virt >> 39L, PageTableFormat.ENTRIES)]);
        if (!*pageEntry) {
            PhysicalAddress addr = allocAndZero(1);
            *pageEntry = (addr | (VirtualPageMasks.PAGE_PRESENT |
                                  VirtualPageMasks.PAGE_WRITABLE));
            printNumber((USize)*pageEntry, 16);
            globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
        }

        /* 1G */
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        pageEntry = (PhysicalAddress *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(
            pageEntry[RING_RANGE_VALUE(virt >> 30L, PageTableFormat.ENTRIES)]);
        if (!*pageEntry) {
            *pageEntry = (allocAndZero(1) | (VirtualPageMasks.PAGE_PRESENT |
                                             VirtualPageMasks.PAGE_WRITABLE));
            printNumber((USize)*pageEntry, 16);
            globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
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
        error(u"Should have received a buffer too small error here!\r\n");
    }

    // Some extra because allocating can create extra descriptors and
    // otherwise
    // exitbootservices will fail (lol)
    mmap.memoryMapSize += mmap.descriptorSize * 2;
    status = globals.st->boot_services->allocate_pages(
        ALLOCATE_ANY_PAGES, LOADER_DATA,
        CEILING_DIV_VALUE(mmap.memoryMapSize, UEFI_PAGE_SIZE),
        (PhysicalAddress *)&mmap.memoryMap);
    if (ERROR(status)) {
        error(u"Could not allocate data for memory map buffer\r\n");
    }

    status = globals.st->boot_services->get_memory_map(
        &mmap.memoryMapSize, mmap.memoryMap, &mmap.mapKey, &mmap.descriptorSize,
        &mmap.descriptorVersion);
    if (ERROR(status)) {
        error(u"Getting memory map failed!\r\n");
    }

    return mmap;
}

// TODO: table 7.10 UEFI spec section 7.2 - 7.2.1 , not fully complete yet I
// think?
bool needsTobeMappedByOS(MemoryType type) {
    switch (type) {
    case RUNTIME_SERVICES_DATA:
        //    case ACPI_RECLAIM_MEMORY:
        //    case ACPI_MEMORY_NVS:
        //    case PAL_CODE:
        return true;
    default:
        return false;
    }
}
