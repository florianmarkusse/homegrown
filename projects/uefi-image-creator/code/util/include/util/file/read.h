#ifndef UTIL_FILE_READ_H
#define UTIL_FILE_READ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "file-status.h"       // for flo_FileStatus
#include "shared/allocator/arena.h" // for Arena
#include "util/text/string.h"  // for string
#include <stdint.h>            // for U64

flo_FileStatus flo_readFile(U8 *srcPath, string *buffer, Arena *perm);
U64 flo_getFileSize(int fd);

#ifdef __cplusplus
}
#endif

#endif
