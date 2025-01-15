#include "image-builder/configuration.h"
#include "image-builder/gpt.h"
#include "image-builder/mbr.h"
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
#include <unistd.h>

static U8 *efiFilePath = "BOOTX64.EFI";
static U8 *kernelFilePath = "kernel.bin";

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

        return (File){0};
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

        return (File){0};
    }
    result.size = buf.st_size;

    return result;
}

static constexpr auto ARGS_SIZE = 3;

int main(int argc, char **argv) {
    if (argc != ARGS_SIZE) {
        PFLUSH_AFTER(STDERR) {
            PERROR(STRING("Program should be called with "));
            PERROR(ARGS_SIZE);
            PERROR(STRING(" arguments:\n"));
            PERROR(STRING_LEN(argv[0], strlen(argv[0])));
            PERROR(STRING(" [efi-file-location] [kernel-file-location]"));
        }
    }

    efiFileInfo = openFile(argv[1]);
    if (efiFileInfo.size == 0) {
        return 1;
    }

    kernelFileInfo = openFile(argv[2]);
    if (kernelFileInfo.size == 0) {
        return 1;
    }

    setConfiguration(efiFileInfo.size, kernelFileInfo.size);

    char *dataBuffer =
        mmap(nullptr, configuration.totalImageSizeBytes, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (dataBuffer == MAP_FAILED) {
        PFLUSH_AFTER(STDERR) {
            PERROR(STRING("Failed to allocate memory for EFI image!\n"));
        }
        return 1;
    }

    writeMBR(dataBuffer);
    writeGPTs(dataBuffer);
    if (!writeEFISystemPartition(dataBuffer, efiFileInfo.fileDescriptor,
                                 efiFileInfo.size)) {
        return 1;
    }

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
        return 1;
    }

    flushBufferWithFileDescriptor(fileDescriptor, dataBuffer,
                                  configuration.totalImageSizeBytes);
}
