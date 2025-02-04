#ifndef EFI_ERROR_H
#define EFI_ERROR_H

#include "platform-abstraction/log.h"
#include "shared/macros.h"

void waitKeyThenReset();

#define EXIT_WITH_MESSAGE                                                      \
    for (auto MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                              \
         MACRO_VAR(i) = 1, flushStandardBuffer(), waitKeyThenReset())

#endif
