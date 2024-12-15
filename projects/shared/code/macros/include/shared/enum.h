#ifndef SHARED_ENUM_H
#define SHARED_ENUM_H

#include "shared/macros.h"
#include "shared/text/string.h"

#define ENUM_TO_STRING(NAME, ...) STRING(STRINGIFY(NAME)),
#define ENUM_FROM_STRING(NAME, ...)                                            \
    if (stringEquals(string, STRING(STRINGIFY(NAME)))) {                       \
        return NAME;                                                           \
    }

#define PLUS_ONE(...) +1
#define ENUM_VALUES_VARIANT(NAME, VALUE) NAME = (VALUE),
#define ENUM_STANDARD_VARIANT(NAME) NAME,

#endif
