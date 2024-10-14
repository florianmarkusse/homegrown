#ifndef UTIL_FILE_READ_H
#define UTIL_FILE_READ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "file-status.h"       // for flo_FileStatus
#include "shared/allocator/arena.h" // for flo_arena
#include "util/text/string.h"  // for flo_string
#include <stdint.h>            // for uint64_t

flo_FileStatus flo_readFile(char *srcPath, flo_string *buffer, flo_arena *perm);
uint64_t flo_getFileSize(int fd);

#ifdef __cplusplus
}
#endif

#endif
