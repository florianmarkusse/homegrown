#include "platform-abstraction/efi.h"
#include "platform-abstraction/log.h"
#include "shared/text/string.h"

void initArchitecture() {
    KFLUSH_AFTER {
        INFO(STRING("Hello I am initing my cpu\n"));
        INFO(STRING("Hello I am initing my cpu\n"));
    }
}
