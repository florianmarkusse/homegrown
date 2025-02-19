#include "os-loader/memory/boot-functions.h"

#include "efi/error.h"
#include "efi/firmware/simple-text-output.h"
#include "efi/firmware/system.h"
#include "efi/globals.h"
#include "efi/memory.h"
#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical/allocation.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "x86/memory/definitions.h"

PhysicalAddress allocAndZero(USize numPages) {
    PhysicalAddress page = allocate4KiBPage(numPages);
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    memset((void *)page, 0, numPages * UEFI_PAGE_SIZE);
    return page;
}

MemoryInfo getMemoryInfo() {
    MemoryInfo mmap = {0};

    // Call GetMemoryMap with initial buffer size of 0 to retrieve the
    // required buffer size
    Status status = globals.st->boot_services->get_memory_map(
        &mmap.memoryMapSize, mmap.memoryMap, &mmap.mapKey, &mmap.descriptorSize,
        &mmap.descriptorVersion);

    if (status != BUFFER_TOO_SMALL) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING(
                "Should have received a buffer too small error here!\n"));
        }
    }

    // Some extra because allocating can create extra descriptors and
    // otherwise
    // exitbootservices will fail (lol)
    mmap.memoryMapSize += mmap.descriptorSize * 2;
    status = globals.st->boot_services->allocate_pages(
        ALLOCATE_ANY_PAGES, LOADER_DATA,
        CEILING_DIV_VALUE(mmap.memoryMapSize, UEFI_PAGE_SIZE),
        (PhysicalAddress *)&mmap.memoryMap);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not allocate data for memory map buffer\n"));
    }

    status = globals.st->boot_services->get_memory_map(
        &mmap.memoryMapSize, mmap.memoryMap, &mmap.mapKey, &mmap.descriptorSize,
        &mmap.descriptorVersion);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Getting memory map failed!\n"));
    }

    return mmap;
}
