
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "image-builder/configuration.h"
#include "posix/log.h"
#include "shared/assert.h"
#include "shared/log.h"
#include "shared/types/types.h"
#include "shared/text/string.h"

bool writeDataPartition(U8 *fileBuffer, int kernelfd, U64 kernelSizeBytes) {
    fileBuffer +=
        configuration.dataPartitionStartLBA * configuration.LBASizeBytes;

    for (U8 *exclusiveEnd = fileBuffer + kernelSizeBytes;
         fileBuffer < exclusiveEnd;) {
        I64 partialBytesRead =
            read(kernelfd, fileBuffer, (U64)(exclusiveEnd - fileBuffer));
        if (partialBytesRead < 0) {
            ASSERT(false);
            PFLUSH_AFTER(STDERR) {
                PERROR((STRING("Failed to read bytes from kernel file to "
                               "write to data partition!\n")));
                PERROR(STRING("Error code: "));
                PERROR(errno, NEWLINE);
                PERROR(STRING("Error message: "));
                U8 *errorString = strerror(errno);
                PERROR(STRING_LEN(errorString, strlen(errorString)), NEWLINE);
            }
            return false;
        } else {
            fileBuffer += partialBytesRead;
        }
    }

    return true;
}
