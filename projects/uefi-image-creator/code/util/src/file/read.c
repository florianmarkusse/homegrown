#include "util/file/read.h"
#include "interoperation/types.h"    // for NULL_ON_FAIL
#include "shared/allocator/arena.h"  // for FLO_NEW, Arena
#include "shared/allocator/macros.h" // for NULL_ON_FAIL
#include "util/file/file-status.h"   // for FILE_CANT_ALLOCATE, FILE_CANT_OPEN
#include "util/log.h"                // for FLO_ERROR, FLO_LOG_CHOOSER_IMPL_2
#include "util/text/string.h"        // for STRING, string
#include <errno.h>                   // for errno
#include <linux/fs.h>                // for BLKGETSIZE64
#include <stddef.h>                  // for U64
#include <stdio.h>                   // for fclose, perror, NULL, fopen, fread
#include <string.h>                  // for strerror
#include <sys/ioctl.h>               // for ioctl
#include <sys/stat.h>                // for stat, fstat, S_ISBLK, S_ISREG

flo_FileStatus flo_readFile(U8 *srcPath, string *buffer, Arena *perm) {
    FILE *srcFile = fopen(srcPath, "rbe");
    if (srcFile == NULL) {
        FLO_FLUSH_AFTER(FLO_STDERR) {
            FLO_ERROR(STRING("Failed to open file: "));
            FLO_ERROR(srcPath, FLO_NEWLINE);
            FLO_ERROR(STRING("Error code: "));
            FLO_ERROR(errno, FLO_NEWLINE);
            FLO_ERROR(STRING("Error message: "));
            FLO_ERROR(strerror(errno), FLO_NEWLINE);
        }
        return FILE_CANT_OPEN;
    }

    fseek(srcFile, 0, SEEK_END);
    U64 dataLen = ftell(srcFile);
    rewind(srcFile);

    (*buffer).buf = NEW(perm, U8, dataLen, NULL_ON_FAIL);
    if ((*buffer).buf == NULL) {
        FLO_FLUSH_AFTER(FLO_STDERR) {
            FLO_ERROR((STRING("Failed to allocate memory for file ")));
            FLO_ERROR(srcPath, FLO_NEWLINE);
        }
        fclose(srcFile);
        return FILE_CANT_ALLOCATE;
    }

    U64 result = fread((*buffer).buf, 1, dataLen, srcFile);
    if (result != dataLen) {
        FLO_FLUSH_AFTER(FLO_STDERR) {
            FLO_ERROR((STRING("Failed to read the file contents of ")));
            FLO_ERROR(srcPath, FLO_NEWLINE);
        }
        fclose(srcFile);
        return FILE_CANT_READ;
    }

    (*buffer).len = dataLen;

    fclose(srcFile);
    return FILE_SUCCESS;
}

U64 flo_getFileSize(int fd) {
    struct stat st;

    if (fstat(fd, &st) < 0) {
        perror("fstat");
        return -1;
    }
    if (S_ISBLK(st.st_mode)) {
        unsigned long long bytes;
        if (ioctl(fd, BLKGETSIZE64, &bytes) != 0) {
            perror("ioctl");
            return -1;
        }
        return bytes;
    } else if (S_ISREG(st.st_mode))
        return st.st_size;

    return -1;
}
