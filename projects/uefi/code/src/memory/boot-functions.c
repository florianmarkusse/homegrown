#include "memory/boot-functions.h"
#include "globals.h"
#include "memory-definitions.h"
#include "memory/standard.h"
#include "printing.h"

CEfiPhysicalAddress allocAndZero(CEfiUSize numPages) {
    CEfiPhysicalAddress page = 0;
    CEfiStatus status = globals.st->boot_services->allocate_pages(
        C_EFI_ALLOCATE_ANY_PAGES, C_EFI_LOADER_DATA, numPages, &page);
    if (C_EFI_ERROR(status)) {
        error(u"unable to allocate pages!\r\n");
    }

    memset((void *)page, 0, numPages * PAGE_SIZE);
    return page;
}

static CEfiPhysicalAddress bumpMemory = KERNEL_SPACE_START;
void mapMemoryAt(CEfiU64 phys, CEfiU64 virt, CEfiU64 size) {
    /* is this a canonical address? We handle virtual memory up to 256TB */
    if (!globals.level4PageTable ||
        ((virt >> 48L) != 0x0000 && (virt >> 48L) != 0xffff)) {
        error(u"Incorrect address mapped or no page table set up yet!\r\n");
    }

    CEfiU64 end = virt + size;
    CEfiU64 *pageEntry = C_EFI_NULL;
    /* walk the page tables and add the missing pieces */
    for (virt &= ~(PAGE_MASK), phys &= ~(PAGE_MASK); virt < end;
         virt += PAGE_SIZE, phys += PAGE_SIZE) {
        /* 512G */
        pageEntry = &(globals.level4PageTable[(virt >> 39L) & PAGE_ENTRY_MASK]);
        if (!*pageEntry) {
            CEfiPhysicalAddress addr = allocAndZero(1);
            *pageEntry = (addr | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 1G */
        pageEntry = (CEfiPhysicalAddress *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> 30L) & PAGE_ENTRY_MASK]);
        if (!*pageEntry) {
            *pageEntry = (allocAndZero(1) | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 2M  */
        pageEntry = (CEfiPhysicalAddress *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> 21L) & PAGE_ENTRY_MASK]);
        if (!*pageEntry) {
            *pageEntry = (allocAndZero(1) | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 4K */
        pageEntry = (CEfiPhysicalAddress *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> 12L) & PAGE_ENTRY_MASK]);
        /* if this page is already mapped, that means the kernel has invalid,
         * overlapping segments */
        if (!*pageEntry) {
            *pageEntry =
                // TODO: Remove WRITE_THROUGH once figured out what is necessary
                // for firmware to work.
                phys | (PAGE_PRESENT | PAGE_WRITABLE);
        } else {
            error(u"This should not happen!\r\n");
        }
    }
}

// TODO: Need to create a safeguord for overflow ...
void mapMemory(CEfiU64 phys, CEfiU64 size) {
    mapMemoryAt(phys, bumpMemory, size);
    bumpMemory += size;
    bumpMemory += bumpMemory & PAGE_MASK ? PAGE_SIZE : 0;
}

MemoryInfo getMemoryInfo() {
    MemoryInfo mmap = {0};

    // Call GetMemoryMap with initial buffer size of 0 to retrieve the
    // required buffer size
    CEfiStatus status = globals.st->boot_services->get_memory_map(
        &mmap.memoryMapSize, mmap.memoryMap, &mmap.mapKey, &mmap.descriptorSize,
        &mmap.descriptorVersion);

    if (status != C_EFI_BUFFER_TOO_SMALL) {
        error(u"Should have received a buffer too small error here!\r\n");
    }

    // Some extra because allocating can create extra descriptors and
    // otherwise
    // exitbootservices will fail (lol)
    mmap.memoryMapSize += mmap.descriptorSize * 2;
    status = globals.st->boot_services->allocate_pages(
        C_EFI_ALLOCATE_ANY_PAGES, C_EFI_LOADER_DATA,
        BYTES_TO_PAGES(mmap.memoryMapSize), &mmap.memoryMap);
    if (C_EFI_ERROR(status)) {
        error(u"Could not allocate data for memory map buffer\r\n");
    }

    status = globals.st->boot_services->get_memory_map(
        &mmap.memoryMapSize, mmap.memoryMap, &mmap.mapKey, &mmap.descriptorSize,
        &mmap.descriptorVersion);
    if (C_EFI_ERROR(status)) {
        error(u"Getting memory map failed!\r\n");
    }

    return mmap;
}

// TODO: table 7.10 UEFI spec section 7.2 - 7.2.1 , not fully complete yet I
// think?
bool needsTobeMappedByOS(CEfiMemoryType type) {
    switch (type) {
    case C_EFI_RUNTIME_SERVICES_DATA:
        //    case C_EFI_ACPI_RECLAIM_MEMORY:
        //    case C_EFI_ACPI_MEMORY_NVS:
        //    case C_EFI_PAL_CODE:
        return true;
    default:
        return false;
    }
}
