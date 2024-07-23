#include "acpi/signatures.h"

static string signatures[] = {STRING("FACP"), STRING("APIC"), STRING("HPET"),
                              STRING("MCFG"), STRING("WAET")};

ACPITable ACPITablesToEnum(string signature) {
    for (U64 i = 0; i < ERROR_AND_NUM_TABLES; i++) {
        if (stringEquals(signature, signatures[i])) {
            return (ACPITable)i;
        }
    }
    return ERROR_AND_NUM_TABLES;
}
