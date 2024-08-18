#include "util/file/read.h"
#include "util/file/file-status.h" // for FILE_CANT_ALLOCATE, FILE_CANT_OPEN
#include "util/log.h"              // for ERROR, LOG_CHOOSER_IMPL_2
#include "util/memory/arena.h"     // for NEW, arena
#include "util/memory/macros.h"    // for NULL_ON_FAIL
#include "util/text/string.h"      // for STRING, string
#include <errno.h>                 // for errno
#include <fcntl.h>
#include <linux/fs.h>
#include <stddef.h> // for NULL, ptrdiff_t
#include <stdio.h>  // for fclose, fopen, fread, fseek, ftell
#include <string.h> // for strerror
#include <sys/ioctl.h>
#include <sys/stat.h>

FileStatus readFile(char *srcPath, string *buffer, Arena *perm) {
    FILE *srcFile = fopen(srcPath, "rbe");
    if (srcFile == NULL) {
        FLUSH_AFTER(STDERR) {
            ERROR((STRING("Failed to open file: ")));
            ERROR(srcPath, NEWLINE);
            ERROR("Error code: ");
            ERROR(errno, NEWLINE);
            ERROR("Error message: ");
            ERROR(strerror(errno), NEWLINE);
        }
        return FILE_CANT_OPEN;
    }

    fseek(srcFile, 0, SEEK_END);
    ptrdiff_t dataLen = ftell(srcFile);
    rewind(srcFile);

    (*buffer).buf = NEW(perm, unsigned char, dataLen, NULL_ON_FAIL);
    if ((*buffer).buf == NULL) {
        FLUSH_AFTER(STDERR) {
            ERROR((STRING("Failed to allocate memory for file ")));
            ERROR(srcPath, NEWLINE);
        }
        fclose(srcFile);
        return FILE_CANT_ALLOCATE;
    }

    ptrdiff_t result = fread((*buffer).buf, 1, dataLen, srcFile);
    if (result != dataLen) {
        FLUSH_AFTER(STDERR) {
            ERROR((STRING("Failed to read the file contents of ")));
            ERROR(srcPath, NEWLINE);
        }
        fclose(srcFile);
        return FILE_CANT_READ;
    }

    (*buffer).len = dataLen;

    fclose(srcFile);
    return FILE_SUCCESS;
}

uint64_t getFileSize(int fd) {
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
