#include "util/file/path.h"
#include "util/memory/arena.h" // for FLO_NEW, flo_arena
#include "util/text/string.h"  // for flo_firstOccurenceOfFrom, flo_string
#include <stddef.h>            // for ptrdiff_t
#include <string.h>            // for memcpy
#include <sys/stat.h>          // for mkdir

static constexpr auto FULL_ACCESS = 0700;

void flo_createPath(flo_string fileLocation, flo_arena scratch) {
    ptrdiff_t currentIndex = 0;
    ptrdiff_t slashIndex =
        flo_firstOccurenceOfFrom(fileLocation, '/', currentIndex);
    if (slashIndex >= 0) {
        char *dirPath = FLO_NEW(&scratch, char, fileLocation.len + 1);
        memcpy(dirPath, fileLocation.buf, fileLocation.len);
        dirPath[fileLocation.len] = '\0';

        while (slashIndex > 0) {
            dirPath[slashIndex] =
                '\0'; // Temporarily terminate the string at the next slash
            mkdir(dirPath, FULL_ACCESS); // Create the directory
            dirPath[slashIndex] = '/';

            currentIndex = slashIndex + 1;
            slashIndex =
                flo_firstOccurenceOfFrom(fileLocation, '/', currentIndex);
        }
    }
}
