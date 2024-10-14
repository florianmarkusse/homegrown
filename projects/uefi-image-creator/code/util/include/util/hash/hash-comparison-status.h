#ifndef UTIL_HASH_HASH_COMPARISON_STATUS_H
#define UTIL_HASH_HASH_COMPARISON_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "shared/text/string.h"

typedef enum {
    HASH_COMPARISON_SUCCESS,
    HASH_COMPARISON_DIFFERENT_SIZES,
    HASH_COMPARISON_DIFFERENT_CONTENT,
    HASH_COMPARISON_NUM_STATUS
} flo_HashComparisonStatus;

string flo_hashComparisonStatusToString(flo_HashComparisonStatus status);

#ifdef __cplusplus
}
#endif

#endif
