#include "abstraction/virtual/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/text/string.h"
#include "shared/types/types.h"
#include "x86-virtual.h"
#include "x86/configuration/cpu2.h"
#include "x86/memory/pat.h"

static string patEncodingToString[PAT_ENCODING_COUNT] = {
    STRING("Uncachable (UC)"),        STRING("Write Combining (WC)"),
    STRING("Reserved 1, don't use!"), STRING("Reserved 2, don't use!"),
    STRING("Write Through (WT)"),     STRING("Write Protected (WP)"),
    STRING("Write Back (WB)"),        STRING("Uncached (UC-)"),
};

void appendVirtualMemoryManagerStatus() {
    KLOG(STRING("CR3/root page table address is: "));
    KLOG((void *)level4PageTable, NEWLINE);

    PAT patValues = {.value = rdmsr(PAT_LOCATION)};
    KLOG(STRING("PAT MSR set to:\n"));
    for (U8 i = 0; i < 8; i++) {
        KLOG(STRING("PAT "));
        KLOG(i);
        KLOG(STRING(": "));
        KLOG(patEncodingToString[patValues.pats[i].pat], NEWLINE);
    }
}
