#include "util/log.h"
#include "util/text/string.h" // for FLO_STRING

int main() {
    FLO_FLUSH_AFTER(FLO_STDOUT) { FLO_LOG(FLO_STRING("hi there\n")); }

    return 0;
}
