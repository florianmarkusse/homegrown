#include "platform-abstraction/physical/allocation.h"

#include "efi-to-kernel/memory/descriptor.h"
#include "efi/error.h"
#include "efi/firmware/base.h"
#include "efi/firmware/system.h"
#include "efi/globals.h"
#include "platform-abstraction/log.h"
#include "shared/types/types.h"

U64 allocate4KiBPage(U64 numPages) {
    U64 address;

    Status status = globals.st->boot_services->allocate_pages(
        ALLOCATE_ANY_PAGES, LOADER_DATA, numPages, &address);
    if (EFI_ERROR(status)) {
        KFLUSH_AFTER { ERROR(STRING("Could not allocate 4KiB page\n")); }
        waitKeyThenReset();
    }

    return address;
}
