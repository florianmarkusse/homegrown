#include "x86/cpu/status/core.h"

#include "platform-abstraction/log.h"
#include "shared/text/string.h"
#include "shared/enum.h"
#include "shared/text/converter.h"

static string faultToString[CPU_FAULT_COUNT] = {CPU_FAULT_ENUM(ENUM_TO_STRING)};

void appendInterrupt(Fault fault) {
    KLOG(STRING("Fault #: "));
    KLOG(fault);
    KLOG(STRING("\tMsg: "));
    KLOG(stringWithMinSizeDefault(faultToString[fault], 30));
}
