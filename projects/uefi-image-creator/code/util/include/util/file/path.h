#ifndef UTIL_FILE_PATH_H
#define UTIL_FILE_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/memory/arena.h"
#include "util/text/string.h"

void flo_createPath(flo_string fileLocation, flo_arena scratch);

#ifdef __cplusplus
}
#endif

#endif
