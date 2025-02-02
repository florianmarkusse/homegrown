#ifndef POSIX_FILE_PATH_H
#define POSIX_FILE_PATH_H

#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h"

void createPath(string fileLocation, Arena scratch);

#endif
