#include "shared/hash/hash-comparison-status.h"

static string hashComparisonStatusStrings[HASH_COMPARISON_NUM_STATUS] = {
    STRING("Success"), STRING("Collections have different sizes"),
    STRING("Collections have different content")};

string hashComparisonStatusToString(HashComparisonStatus status) {
    if (status >= 0 && status < HASH_COMPARISON_NUM_STATUS) {
        return hashComparisonStatusStrings[status];
    }
    return STRING("Unknown hash comparison status code!");
}
