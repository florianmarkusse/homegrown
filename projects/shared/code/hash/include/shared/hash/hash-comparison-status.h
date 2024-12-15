#ifndef SHARED_HASH_HASH_COMPARISON_STATUS_H
#define SHARED_HASH_HASH_COMPARISON_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "shared/enum.h"
#include "shared/text/string.h"

#define HASH_COMPARISON_STATUS_ENUM(VARIANT)                                   \
    VARIANT(HASH_COMPARISON_SUCCESS)                                           \
    VARIANT(HASH_COMPARISON_DIFFERENT_SIZES)                                   \
    VARIANT(HASH_COMPARISON_DIFFERENT_CONTENT)

typedef enum {
    HASH_COMPARISON_STATUS_ENUM(ENUM_STANDARD_VARIANT)
} HashComparisonStatus;
static constexpr auto HASH_COMPARISON_STATUS_COUNT =
    (0 HASH_COMPARISON_STATUS_ENUM(PLUS_ONE));

extern string hashComparisonStatusToString[HASH_COMPARISON_STATUS_COUNT];

#ifdef __cplusplus
}
#endif

#endif
