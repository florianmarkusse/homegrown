#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "image-builder/configuration.h"
#include "image-builder/gpt.h"
#include "image-builder/mbr.h"
#include "image-builder/partitions/data.h"
#include "image-builder/partitions/efi.h"
#include "posix/file/file-status.h"
#include "posix/log.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/sizes.h"
#include "shared/text/parser.h"
#include "shared/text/string.h"
#include "shared/types/types.h"

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

int createUEFIImage() {
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
                                 efiFileInfo.size, kernelFileInfo.size)) {
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

static constexpr char EFI_FILE_FLAG[] = "--efi";
static constexpr char KERNEL_FILE_FLAG[] = "--kernel";
static constexpr char PHYSICAL_BOUNDARY_FLAG[] = "--phys";
static constexpr char OPTIMAL_TRANSFER_FLAG[] = "--optimal-transfer-length";
static constexpr char LBA_FLAG[] = "--lba";

static char *efiFile = 0;
static char *kernelFile = 0;
static U32 physicalBoundary = 0;
static U32 optimalTransferLengthGranularity = 0;

int main(int argc, char **argv) {
    time_t programStartTime = time(NULL);
    if (__builtin_setjmp(errorHandler)) {
        struct stat fileStat;
        if (stat(configuration.imageName, &fileStat) == -1) {
            PFLUSH_AFTER(STDERR) {
                PERROR(STRING("Could not stat file: "));
                PERROR(STRING_LEN(configuration.imageName,
                                  strlen(configuration.imageName)),
                       NEWLINE);
                PERROR(STRING("Aborting error handler!\n"));
                PERROR(STRING("Perform manual checks to decide what to do with "
                              "the file.\n"));
            }
            return 1;
        }
        time_t imageModifiedTime = fileStat.st_mtime;
        if (programStartTime > imageModifiedTime) {
            PFLUSH_AFTER(STDERR) {
                PERROR(STRING("File: "));
                PERROR(STRING_LEN(configuration.imageName,
                                  strlen(configuration.imageName)));
                PERROR(STRING(
                    " was not modified by the program, no cleanup done.\n"));
            }
        } else {
            PFLUSH_AFTER(STDERR) {
                PERROR(STRING("File: "));
                PERROR(STRING_LEN(configuration.imageName,
                                  strlen(configuration.imageName)));
                PERROR(STRING(" was modified by the program removing...\n"));
            }
            unlink(configuration.imageName);
        }

        return 1;
    }

    for (int i = 1; i < argc; i += 2) {
        char *flag = argv[i];
        if (!strcmp(flag, EFI_FILE_FLAG)) {
            efiFile = argv[i + 1];
        }
        if (!strcmp(flag, KERNEL_FILE_FLAG)) {
            kernelFile = argv[i + 1];
        }
        if (!strcmp(flag, PHYSICAL_BOUNDARY_FLAG)) {
            string physString = STRING_LEN(argv[i + 1], strlen(argv[i + 1]));
            physicalBoundary = parseU32(physString, 10);
        }
        if (!strcmp(flag, OPTIMAL_TRANSFER_FLAG)) {
            string optimalTransferString =
                STRING_LEN(argv[i + 1], strlen(argv[i + 1]));
            optimalTransferLengthGranularity =
                parseU32(optimalTransferString, 10);
        }
        if (!strcmp(flag, LBA_FLAG)) {
            string lbaString = STRING_LEN(argv[i + 1], strlen(argv[i + 1]));
            configuration.LBASizeBytes = parseU16(lbaString, 10);
        }
    }

    if (!kernelFile || !efiFile) {
        PFLUSH_AFTER(STDERR) {
            PERROR(STRING("Usage:\n\t"));
            PERROR(STRING_LEN(argv[0], strlen(argv[0])), NEWLINE);

            PERROR(STRING("\t"));
            PERROR(STRING_LEN(EFI_FILE_FLAG, strlen(EFI_FILE_FLAG)));
            PERROR(STRING(" <efi file location>\n"));

            PERROR(STRING("\t"));
            PERROR(STRING_LEN(KERNEL_FILE_FLAG, strlen(KERNEL_FILE_FLAG)));
            PERROR(STRING(" <kernel file location>\n"));

            PERROR(STRING("\t["));
            PERROR(STRING_LEN(PHYSICAL_BOUNDARY_FLAG,
                              strlen(PHYSICAL_BOUNDARY_FLAG)));
            PERROR(STRING(" <physical boundary of device in bytes>]\n"));

            PERROR(STRING("\t["));
            PERROR(STRING_LEN(OPTIMAL_TRANSFER_FLAG,
                              strlen(OPTIMAL_TRANSFER_FLAG)));
            PERROR(STRING(" <optimal transfer length of device in bytes>]\n"));

            PERROR(STRING("\t["));
            PERROR(STRING_LEN(LBA_FLAG, strlen(LBA_FLAG)));
            PERROR(STRING(" <logical block size in bytes>]\n"));
        }
        __builtin_longjmp(errorHandler, 1);
    }

    efiFileInfo = openFile(efiFile);
    kernelFileInfo = openFile(kernelFile);

    U32 GPTTableAlignmentSizeBytes = 1 * MiB;
    if (physicalBoundary || optimalTransferLengthGranularity) {
        GPTTableAlignmentSizeBytes =
            MAX(physicalBoundary, optimalTransferLengthGranularity);
    }
    setConfiguration(efiFileInfo.size, kernelFileInfo.size,
                     GPTTableAlignmentSizeBytes);

    return createUEFIImage();
}
