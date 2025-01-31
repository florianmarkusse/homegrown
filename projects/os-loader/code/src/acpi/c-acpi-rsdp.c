#include "os-loader/acpi/c-acpi-rdsp.h" // for RSDP_REVISION_2, CAcpiRSDPV1, CAcpi...
#include "os-loader/acpi/configuration-table.h" // for ConfigurationTable
#include "os-loader/acpi/guid.h"
#include "platform-abstraction/memory/manipulation.h" // for memcmp
#include "shared/types/types.h" // for USize, U8, nullptr, U16, U64
#include "shared/uuid.h"        // for Guid, ACPI_TABLE_GUID, EFI_ACPI_20_...

bool acpi_checksum(void *ptr, U64 size) {
    U8 sum = 0, *_ptr = ptr;
    for (USize i = 0; i < size; i++) {
        sum += _ptr[i];
    }
    return sum == 0;
}

typedef struct {
    UUID guid;
    USize size;
    RSDPRevision revision;
    U16 *string;
} RSDPStruct;

static RSDPStruct possibleRsdps[2] = {
    {.guid = ACPI_TABLE_GUID,
     .size = sizeof(CAcpiRSDPV1),
     .revision = RSDP_REVISION_1,
     .string = u"RSDP REVISION 1"},
    {.guid = EFI_ACPI_20_TABLE_GUID,
     .size = sizeof(CAcpiRSDPV2),
     .revision = RSDP_REVISION_2,
     .string = u"RSDP REVISION 2"},
};
#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))
static constexpr auto POSSIBLE_RSDP_NUM = COUNTOF(possibleRsdps);

RSDPResult getRSDP(USize tableEntries, ConfigurationTable *tables) {
    RSDPResult rsdp = {.rsdp = nullptr};
    for (USize i = 0; i < tableEntries; i++) {
        ConfigurationTable *cur_table = &tables[i];

        for (USize i = 0; i < POSSIBLE_RSDP_NUM; i++) {
            if (memcmp(&cur_table->vendor_guid, &possibleRsdps[i].guid,
                       sizeof(UUID)) != 0) {
                continue;
            }
            if (!acpi_checksum(cur_table->vendor_table,
                               possibleRsdps[i].size)) {
                continue;
            }

            // We want to return the newest version if it exists rather then
            // returning the older version. We need to add a check for that
            // since the table entries are not in the same order for all EFI
            // systems.
            rsdp.rsdp = (void *)cur_table->vendor_table;
            rsdp.revision = possibleRsdps[i].revision;
            if (rsdp.revision == RSDP_REVISION_2) {
                return rsdp;
            }
        }
    }

    return rsdp;
}
