#ifndef POSIX_FILE_FILE_STATUS_H
#define POSIX_FILE_FILE_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "shared/text/string.h"

typedef enum {
    FILE_SUCCESS,
    FILE_CANT_OPEN,
    FILE_CANT_ALLOCATE,
    FILE_CANT_READ,
    FILE_NUM_STATUS
} FileStatus;

static string fileStatusStrings[FILE_NUM_STATUS] = {
    STRING("Success"),
    STRING("Cannot open file"),
    STRING("Cannot allocate memory"),
    STRING("Cannot read file"),
};

// Not always used, but very handy for those that actually do want readable
// error codes.
__attribute__((unused)) static string
flo_fileStatusToString(FileStatus status) {
    if (status >= 0 && status < FILE_NUM_STATUS) {
        return fileStatusStrings[status];
    }
    return STRING("Unknown file status code!");
}

#ifdef __cplusplus
}
#endif

#endif
