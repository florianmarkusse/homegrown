#ifndef EFI_ERROR_H
#define EFI_ERROR_H

#include "efi/firmware/base.h"
#include "abstraction/log.h"
#include "shared/macros.h"

void waitKeyThenReset();

#define EXIT_WITH_MESSAGE                                                      \
    for (auto MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                              \
         MACRO_VAR(i) = 1, flushStandardBuffer(), waitKeyThenReset())

#define EXIT_WITH_MESSAGE_IF(status)                                           \
    if (EFI_ERROR(status))                                                     \
    EXIT_WITH_MESSAGE

#endif
