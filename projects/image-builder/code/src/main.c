#include "image-builder/configuration.h"
#include "image-builder/gpt.h"
#include "image-builder/mbr.h"
#include "platform-abstraction/log.h"
#include "posix/file/file-status.h"
#include "posix/log.h"
#include "shared/log.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/macros.h"
#include "shared/memory/sizes.h"
#include "shared/text/string.h"
#include "shared/types/types.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

static void *resourceCleanup[5];

typedef MAX_LENGTH_ARRAY(FILE *) FILEPtr_max_a;

static constexpr auto MAX_OPEN_FILES = 64;
FILE *openFilesBuf[MAX_OPEN_FILES];
FILEPtr_max_a openedFiles = {
    .buf = openFilesBuf, .cap = MAX_OPEN_FILES, .len = 0};

static constexpr auto MEMORY_CAP = 1 * GiB;

int main(int argc, char **argv) {
    char *begin = mmap(nullptr, MEMORY_CAP, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (begin == MAP_FAILED) {
        PFLUSH_AFTER(STDERR) {
            PERROR(STRING("Failed to allocate memory for arena!\n"));
        }
        return 1;
    }

    Arena arena = (Arena){
        .beg = begin,
        .curFree = begin,
        .end = begin + MEMORY_CAP,
    };

    if (__builtin_setjmp(resourceCleanup)) {
        for (U64 i = 0; i < openedFiles.len; i++) {
            if (fclose(openedFiles.buf[i])) {
                PFLUSH_AFTER(STDERR) {
                    PERROR(STRING("Failed to close FILE*\n"));
                    PERROR(STRING("Error code: "));
                    PERROR(errno, NEWLINE);
                    PERROR(STRING("Error message: "));
                    U8 *errorString = strerror(errno);
                    PERROR(STRING_LEN(errorString, strlen(errorString)),
                           NEWLINE);
                }
            }
        }

        if (munmap(arena.beg, MEMORY_CAP) == -1) {
            PFLUSH_AFTER(STDERR) {
                PERROR((STRING("Failed to unmap memory from"
                               "arena !\n "
                               "Arena Details:\n"
                               "  beg: ")));
                PERROR(arena.beg);
                PERROR((STRING("\n end: ")));
                PERROR(arena.end);
                PERROR((STRING("\n memory capacity: ")));
                PERROR(MEMORY_CAP);
            }
        }
        PFLUSH_AFTER(STDERR) { PERROR((STRING("\nZeroing Arena\n"))); }
        arena = (Arena){0};

        return 1;
    }

    arena.jmp_buf = resourceCleanup;

    setConfiguration();

    U64 fileSizeBytes = configuration.totalImageSizeLBA * configuration.LBASize;
    U8 *dataBuffer = NEW(&arena, U8, fileSizeBytes, ZERO_MEMORY);

    writeMBR(dataBuffer);
    writeGPT(dataBuffer);

    int fileDescriptor =
        open(configuration.imageName, O_CLOEXEC | O_TRUNC | O_CREAT | O_RDWR,
             READ_WRITE_OWNER_READ_OTHERS);
    if (fileDescriptor == -1) {
        PFLUSH_AFTER(STDERR) {
            PERROR((STRING("Failed to open file for writing!\n")));
            PERROR(STRING("Error code: "));
            PERROR(errno, NEWLINE);
            PERROR(STRING("Error message: "));
            U8 *errorString = strerror(errno);
            PERROR(STRING_LEN(errorString, strlen(errorString)), NEWLINE);
        }
    }

    flushBufferWithFileDescriptor(fileDescriptor, dataBuffer, fileSizeBytes);
}
