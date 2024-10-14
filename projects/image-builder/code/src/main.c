#include "interoperation/memory/sizes.h"
#include "interoperation/types.h"
#include "util/log.h"
#include "util/text/string.h" // for FLO_STRING
#include <sys/mman.h>

// MBR Partition
typedef struct {
    U8 bootIndicator;
    U8 startingCHS[3];
    U8 osType;
    U8 endingCHS[3];
    U32 startingLBA;
    U32 sizeLBA;
} __attribute__((packed)) MBRPartition;

// Master Boot Record
typedef struct {
    U8 bootCode[440];
    U32 signature;
    U16 unknown;
    MBRPartition partitions[4];
    U16 boot_signature;
} __attribute__((packed)) MBR;

static constexpr auto MEMORY_CAP = 1 * GiB;

int main() {
    char *begin = mmap(NULL, MEMORY_CAP, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (begin == MAP_FAILED) {
        FLO_FLUSH_AFTER(FLO_STDERR) {
            FLO_ERROR(FLO_STRING("Failed to allocate memory!\n"));
        }
        return 1;
    }

    flo_arena arena = (flo_arena){.beg = begin,
                                  .cap = MEMORY_CAP,
                                  .end = begin + (ptrdiff_t)(MEMORY_CAP)};

    /*if (__builtin_setjmp(memoryErrors)) {*/
    /*    if (munmap(arena.beg, arena.cap) == -1) {*/
    /*        FLO_FLUSH_AFTER(FLO_STDERR) {*/
    /*            FLO_ERROR((FLO_STRING("Failed to unmap memory from"*/
    /*                                  "arena !\n "*/
    /*                                  "Arena Details:\n"*/
    /*                                  "  beg: ")));*/
    /*            FLO_ERROR(arena.beg);*/
    /*            FLO_ERROR((FLO_STRING("\n end: ")));*/
    /*            FLO_ERROR(arena.end);*/
    /*            FLO_ERROR((FLO_STRING("\n cap: ")));*/
    /*            FLO_ERROR(arena.cap);*/
    /*            FLO_ERROR((FLO_STRING("\nZeroing Arena regardless.\n")));*/
    /*        }*/
    /*    }*/
    /*    arena.beg = NULL;*/
    /*    arena.end = NULL;*/
    /*    arena.cap = 0;*/
    /*    arena.jmp_buf = NULL;*/
    /*    FLO_ERROR(*/
    /*        (FLO_STRING("Early exit due to error or OOM/overflow in
     * arena!\n")),*/
    /*        FLO_FLUSH);*/
    /*    __builtin_longjmp(fileCloser, 1);*/
    /*}*/
    /*arena.jmp_buf = memoryErrors;*/

    FLO_FLUSH_AFTER(FLO_STDOUT) { FLO_LOG(FLO_STRING("hi there\n")); }

    return 0;
}
