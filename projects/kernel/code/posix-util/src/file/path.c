#include "util/file/path.h"    // for createPath
#include "util/memory/arena.h" // for NEW, arena
#include "util/text/string.h"  // for firstOccurenceOfFrom, string
#include <stddef.h>            // for ptrdiff_t
#include <string.h>            // for memcpy
#include <sys/stat.h>          // for mkdir

#define FULL_ACCESS 0700

void createPath(string fileLocation, Arena scratch) {
    ptrdiff_t currentIndex = 0;
    ptrdiff_t slashIndex =
        firstOccurenceOfFrom(fileLocation, '/', currentIndex);
    if (slashIndex >= 0) {
        char *dirPath = NEW(&scratch, char, fileLocation.len + 1);
        memcpy(dirPath, fileLocation.buf, fileLocation.len);
        dirPath[fileLocation.len] = '\0';

        while (slashIndex > 0) {
            dirPath[slashIndex] =
                '\0'; // Temporarily terminate the string at the next slash
            mkdir(dirPath, FULL_ACCESS); // Create the directory
            dirPath[slashIndex] = '/';

            currentIndex = slashIndex + 1;
            slashIndex = firstOccurenceOfFrom(fileLocation, '/', currentIndex);
        }
    }
}
