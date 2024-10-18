#ifndef POSIX_FILE_PATH_H
#define POSIX_FILE_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h"

void createPath(string fileLocation, Arena scratch);

#ifdef __cplusplus
}
#endif

#endif
