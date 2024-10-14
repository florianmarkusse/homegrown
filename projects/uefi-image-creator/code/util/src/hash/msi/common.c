#include "util/hash/msi/common.h"
#include "interoperation/assert.h" // for ASSERT
#include "shared/allocator/arena.h"
#include "shared/allocator/macros.h"
#include "shared/manipulation/manipulation.h"

/**
 * Written assuming that Arena bumps up! Otherwise the middle case statement
 * where we only do a times 1 alloc does not hold.
 */
void flo_msi_newSet(void *setSlice, U64 size, U64 align, Arena *a) {
    SetSlice *replica = (SetSlice *)setSlice;
    ASSERT(replica->exp > 0);

    if (replica->exp >= 31) {
        ASSERT(false);
        __builtin_longjmp(a->jmp_buf, 1);
    }

    U64 cap = 1 << replica->exp;

    if (replica->buf == NULL) {
        replica->buf = alloc(a, size, align, cap, ZERO_MEMORY);
    } else if (a->beg == replica->buf + size * cap) {
        memset(replica->buf, 0, size * cap);
        alloc(a, size, 1, cap, ZERO_MEMORY);
        replica->exp++;
        replica->len = 0;
    } else {
        void *data = alloc(a, 2 * size, align, cap, ZERO_MEMORY);
        replica->buf = data;
        replica->exp++;
        replica->len = 0;
    }
}
