#ifndef POSIX_FILE_FILE_STATUS_H
#define POSIX_FILE_FILE_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "shared/enum.h"
#include "shared/text/string.h"

#define FILE_STATUS_ENUM(VARIANT)                                              \
    VARIANT(FILE_SUCCESS)                                                      \
    VARIANT(FILE_CANT_OPEN)                                                    \
    VARIANT(FILE_CANT_ALLOCATE)                                                \
    VARIANT(FILE_CANT_READ)

typedef enum { FILE_STATUS_ENUM(ENUM_STANDARD_VARIANT) } FileStatus;
static constexpr auto FILE_STATUS_COUNT = FILE_STATUS_ENUM(PLUS_ONE);

static string fileStatusStrings[FILE_STATUS_COUNT] = {
    FILE_STATUS_ENUM(ENUM_TO_STRING)};

#ifdef __cplusplus
}
#endif

#endif
