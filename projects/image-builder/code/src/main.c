#include "image-builder/configuration.h"
#include "image-builder/gpt.h"
#include "image-builder/mbr.h"
#include "image-builder/partitions/data.h"
#include "image-builder/partitions/efi.h"
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
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void *errorHandler[5];

typedef struct {
    int fileDescriptor;
    U64 size;
} File;

static File efiFileInfo;
static File kernelFileInfo;

File openFile(U8 *name) {
    File result;
    result.fileDescriptor = open(name, O_RDONLY | O_CLOEXEC);
    if (!result.fileDescriptor) {
        PFLUSH_AFTER(STDERR) {
            PERROR(STRING("Could not fopen file "));
            PERROR(STRING_LEN(name, strlen(name)), NEWLINE);
            PLOG(STRING("Error code: "));
            PLOG(errno, NEWLINE);
            PLOG(STRING("Error message: "));
            PLOG(STRING_LEN(strerror(errno), strlen(strerror(errno))), NEWLINE);
        }
        __builtin_longjmp(errorHandler, 1);
    }

    struct stat buf;
    if (fstat(result.fileDescriptor, &buf)) {
        PFLUSH_AFTER(STDERR) {
            PERROR(STRING("Could not fstat file "));
            PERROR(STRING_LEN(name, strlen(name)), NEWLINE);
            PLOG(STRING("Error code: "));
            PLOG(errno, NEWLINE);
            PLOG(STRING("Error message: "));
            PLOG(STRING_LEN(strerror(errno), strlen(strerror(errno))), NEWLINE);
        }
        __builtin_longjmp(errorHandler, 1);
    }
    result.size = buf.st_size;

    return result;
}

static constexpr auto ARGS_SIZE = 3;

int main(int argc, char **argv) {
    time_t programStartTime = time(NULL);
    if (__builtin_setjmp(errorHandler)) {
        struct stat fileStat;
        if (stat(configuration.imageName, &fileStat) == -1) {
            PFLUSH_AFTER(STDERR) {
                PERROR(STRING("Could not stat file: "));
                PERROR(configuration.imageName, NEWLINE);
                PERROR(STRING("Aborting error handler!\n"));
                PERROR(STRING("Perform manual checks to decide what to do with "
                              "the file."));
            }
            return 1;
        }
        time_t imageModifiedTime = fileStat.st_mtime;
        if (programStartTime > imageModifiedTime) {
            PFLUSH_AFTER(STDERR) {
                PERROR(STRING("File: "));
                PERROR(configuration.imageName);
                PERROR(STRING(
                    " was not modified by the program, no cleanup done."));
            }
        } else {
            PFLUSH_AFTER(STDERR) {
                PERROR(STRING("File: "));
                PERROR(configuration.imageName);
                PERROR(STRING(" was modified by the program removing..."));
            }
            unlink(configuration.imageName);
        }

        return 1;
    }

    if (argc != ARGS_SIZE) {
        PFLUSH_AFTER(STDERR) {
            PERROR(STRING("Program should be called with "));
            PERROR(ARGS_SIZE);
            PERROR(STRING(" arguments:\n"));
            PERROR(STRING_LEN(argv[0], strlen(argv[0])));
            PERROR(STRING(" [efi-file-location] [kernel-file-location]"));
        }
        __builtin_longjmp(errorHandler, 1);
    }

    efiFileInfo = openFile(argv[1]);
    kernelFileInfo = openFile(argv[2]);

    setConfiguration(efiFileInfo.size, kernelFileInfo.size);

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
        __builtin_longjmp(errorHandler, 1);
    }

    if (ftruncate(fileDescriptor, configuration.totalImageSizeBytes) == -1) {
        PFLUSH_AFTER(STDERR) {
            PERROR((STRING("Failed to truncate file!\n")));
            PERROR(STRING("Error code: "));
            PERROR(errno, NEWLINE);
            PERROR(STRING("Error message: "));
            U8 *errorString = strerror(errno);
            PERROR(STRING_LEN(errorString, strlen(errorString)), NEWLINE);
        }
        __builtin_longjmp(errorHandler, 1);
        return 1;
    }

    U8 *dataBuffer =
        mmap(NULL, configuration.totalImageSizeBytes, PROT_READ | PROT_WRITE,
             MAP_SHARED, fileDescriptor, 0);
    if (dataBuffer == MAP_FAILED) {
        PFLUSH_AFTER(STDERR) {
            PERROR((STRING("Failed to mmap file!\n")));
            PERROR(STRING("Error code: "));
            PERROR(errno, NEWLINE);
            PERROR(STRING("Error message: "));
            U8 *errorString = strerror(errno);
            PERROR(STRING_LEN(errorString, strlen(errorString)), NEWLINE);
        }
        __builtin_longjmp(errorHandler, 1);
        return 1;
    }

    writeMBR(dataBuffer);
    writeGPTs(dataBuffer);
    if (!writeEFISystemPartition(dataBuffer, efiFileInfo.fileDescriptor,
                                 efiFileInfo.size)) {
        __builtin_longjmp(errorHandler, 1);
        return 1;
    }
    if (!writeDataPartition(dataBuffer, kernelFileInfo.fileDescriptor,
                            kernelFileInfo.size)) {
        __builtin_longjmp(errorHandler, 1);
        return 1;
    }

    return 0;
}
