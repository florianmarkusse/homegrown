#include "memory/boot-functions.h"
#include "efi/c-efi-system.h"
#include "globals.h"
#include "interoperation/memory/definitions.h"
#include "memory/standard.h"
#include "printing.h"

PhysicalAddress allocAndZero(USize numPages) {
    PhysicalAddress page = 0;
    Status status = globals.st->boot_services->allocate_pages(
        ALLOCATE_ANY_PAGES, LOADER_DATA, numPages, &page);
    if (ERROR(status)) {
        error(u"unable to allocate pages!\r\n");
    }

    memset((void *)page, 0, numPages * PAGE_SIZE);
    return page;
}

void mapMemoryAt(U64 phys, U64 virt, U64 size, U64 additionalFlags) {
    /* is this a canonical address? We handle virtual memory up to 256TB */
    if (!globals.level4PageTable ||
        ((virt >> 48L) != 0x0000 && (virt >> 48L) != 0xffff)) {
        error(u"Incorrect address mapped or no page table set up yet!\r\n");
    }

    U64 end = virt + size;
    U64 *pageEntry = NULL;
    /* walk the page tables and add the missing pieces */
    for (virt &= ~(PAGE_MASK), phys &= ~(PAGE_MASK); virt < end;
         virt += PAGE_SIZE, phys += PAGE_SIZE) {
        /* 512G */
        pageEntry =
            &(((PhysicalAddress *)
                   globals.level4PageTable)[(virt >> 39L) & PAGE_ENTRY_MASK]);
        if (!*pageEntry) {
            PhysicalAddress addr = allocAndZero(1);
            *pageEntry = (addr | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 1G */
        pageEntry = (PhysicalAddress *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> 30L) & PAGE_ENTRY_MASK]);
        if (!*pageEntry) {
            *pageEntry = (allocAndZero(1) | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 2M  */
        pageEntry = (PhysicalAddress *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> 21L) & PAGE_ENTRY_MASK]);
        if (!*pageEntry) {
            *pageEntry = (allocAndZero(1) | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 4K */
        pageEntry = (PhysicalAddress *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> 12L) & PAGE_ENTRY_MASK]);
        /* if this page is already mapped, that means the kernel has invalid,
         * overlapping segments */
        if (!*pageEntry) {
            *pageEntry =
                phys | (PAGE_PRESENT | PAGE_WRITABLE) | additionalFlags;
        } //  else {
        //     error(u"This should not happen!\r\n");
        // }
    }
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
        ALLOCATE_ANY_PAGES, LOADER_DATA, BYTES_TO_PAGES(mmap.memoryMapSize),
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
