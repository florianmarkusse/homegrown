#include "acpi/c-acpi-rdsp.h"
#include "efi/c-efi-protocol-simple-text-output.h"
#include "efi/c-efi-system.h"
#include "globals.h"
#include "printing.h"
#include <string.h>

CEfiBool acpi_checksum(void *ptr, size_t size) {
    CEfiU8 sum = 0, *_ptr = ptr;
    for (CEfiUSize i = 0; i < size; i++) {
        sum += _ptr[i];
    }
    return sum == 0;
}

typedef struct {
    CEfiGuid guid;
    CEfiUSize size;
    RSDPRevision revision;
    CEfiChar16 *string;
} RSDPStruct;

static RSDPStruct possibleRsdps[2] = {
    {.guid = C_EFI_ACPI_TABLE_GUID,
     .size = sizeof(CAcpiRSDPV1),
     .revision = RSDP_REVISION_1,
     .string = u"RSDP REVISION 1"},
    {.guid = C_EFI_EFI_ACPI_20_TABLE_GUID,
     .size = sizeof(CAcpiRSDPV2),
     .revision = RSDP_REVISION_2,
     .string = u"RSDP REVISION 2"},
};
#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))
#define POSSIBLE_RSDP_NUM COUNTOF(possibleRsdps)

RSDPResult getRSDP(void) {
    RSDPResult rsdp = {.rsdp = C_EFI_NULL};
    for (size_t i = 0; i < globals.st->number_of_table_entries; i++) {
        CEfiConfigurationTable *cur_table = &globals.st->configuration_table[i];

        for (CEfiUSize i = 0; i < POSSIBLE_RSDP_NUM; i++) {
            if (memcmp(&cur_table->vendor_guid, &possibleRsdps[i].guid,
                       sizeof(CEfiGuid)) != 0) {
                continue;
            }
            if (!acpi_checksum(cur_table->vendor_table,
                               possibleRsdps[i].size)) {
                continue;
            }

            globals.st->con_out->output_string(globals.st->con_out,
                                               u"ACPI: Found ");
            globals.st->con_out->output_string(globals.st->con_out,
                                               possibleRsdps[i].string);
            globals.st->con_out->output_string(globals.st->con_out, u" at ");
            printNumber((CEfiU64)cur_table->vendor_table, 16);
            globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

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

    if (!rsdp.rsdp) {
        error(u"Could not find an RSDP!\r\n");
    }

    return rsdp;
}
