#include "posix/file/read.h"

#include <errno.h>              // for errno

#include "posix/file/file-status.h" // for FILE_CANT_ALLOCATE, FILE_CANT_OPEN
#include "posix/log.h"
#include "shared/log.h"
#include "shared/memory/allocator/arena.h" // for NEW, Arena
#include "shared/memory/allocator/macros.h"
#include "shared/text/string.h" // for STRING, string
#include "shared/types/types.h" // for nullptr_ON_FAIL
#include <linux/fs.h>           // for BLKGETSIZE64
#include <stddef.h>             // for U64
#include <stdio.h>              // for fclose, perror, nullptr, fopen, fread
#include <string.h>             // for strerror
#include <sys/ioctl.h>          // for ioctl
#include <sys/stat.h>           // for stat, fstat, S_ISBLK, S_ISREG

FileStatus readFile(U8 *srcPath, string *buffer, Arena *perm) {
    FILE *srcFile = fopen(srcPath, "rbe");
    if (srcFile == nullptr) {
        PFLUSH_AFTER(STDERR) {
            PERROR(STRING("Failed to open file: "));
            PERROR(srcPath, NEWLINE);
            PERROR(STRING("Error code: "));
            PERROR(errno, NEWLINE);
            PERROR(STRING("Error message: "));
            U8 *errorString = strerror(errno);
            PERROR(STRING_LEN(errorString, strlen(errorString)), NEWLINE);
        }
        return FILE_CANT_OPEN;
    }

    fseek(srcFile, 0, SEEK_END);
    U64 dataLen = ftell(srcFile);
    rewind(srcFile);

    (*buffer).buf = NEW(perm, U8, dataLen, nullptr_ON_FAIL);
    if ((*buffer).buf == nullptr) {
        PFLUSH_AFTER(STDERR) {
            PERROR((STRING("Failed to allocate memory for file ")));
            PERROR(srcPath, NEWLINE);
        }
        fclose(srcFile);
        return FILE_CANT_ALLOCATE;
    }

    U64 result = fread((*buffer).buf, 1, dataLen, srcFile);
    if (result != dataLen) {
        PFLUSH_AFTER(STDERR) {
            PERROR((STRING("Failed to read the file contents of ")));
            PERROR(srcPath, NEWLINE);
        }
        fclose(srcFile);
        return FILE_CANT_READ;
    }

    (*buffer).len = dataLen;

    fclose(srcFile);
    return FILE_SUCCESS;
}

U64 getFileSize(int fd) {
    struct stat st;

    if (fstat(fd, &st) < 0) {
        perror("fstat");
        return -1;
    }
    if (S_ISBLK(st.st_mode)) {
        U64 bytes;
        if (ioctl(fd, BLKGETSIZE64, &bytes) != 0) {
            perror("ioctl");
            return -1;
        }
        return bytes;
    } else if (S_ISREG(st.st_mode))
        return st.st_size;

    return -1;
}
