#ifndef UTIL_FILE_READ_H
#define UTIL_FILE_READ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "file-status.h"       // for FileStatus
#include "util/memory/arena.h" // for arena
#include "util/text/string.h"  // for string

FileStatus readFile(char *srcPath, string *buffer, Arena *perm);
uint64_t getFileSize(int fd);

#ifdef __cplusplus
}
#endif

#endif
