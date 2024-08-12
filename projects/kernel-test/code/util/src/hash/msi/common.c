#include "util/hash/msi/common.h"
#include "util/assert.h" // for ASSERT
#include <string.h>      // for memset

/**
 * Written assuming that arena bumps up! Otherwise the middle case statement
 * where we only do a times 1 alloc does not hold.
 */
void msi_newSet(void *setSlice, ptrdiff_t size, ptrdiff_t align, Arena *a) {
    SetSlice *replica = (SetSlice *)setSlice;
    ASSERT(replica->exp > 0);

    if (replica->exp >= 31) {
        ASSERT(false);
        __builtin_longjmp(a->jmp_buf, 1);
    }

    ptrdiff_t cap = 1 << replica->exp;

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
