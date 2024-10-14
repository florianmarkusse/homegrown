#ifndef UTIL_FILE_PATH_H
#define UTIL_FILE_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "shared/allocator/arena.h"
#include "util/text/string.h"

void flo_createPath(string fileLocation, Arena scratch);

#ifdef __cplusplus
}
#endif

#endif
