#include "util/hash/hash-comparison-status.h"

static flo_string hashComparisonStatusStrings[HASH_COMPARISON_NUM_STATUS] = {
    FLO_STRING("Success"), FLO_STRING("Collections have different sizes"),
    FLO_STRING("Collections have different content")};

flo_string flo_hashComparisonStatusToString(flo_HashComparisonStatus status) {
    if (status >= 0 && status < HASH_COMPARISON_NUM_STATUS) {
        return hashComparisonStatusStrings[status];
    }
    return FLO_STRING("Unknown hash comparison status code!");
}
