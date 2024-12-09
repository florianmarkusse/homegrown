#include "kernel/log/init.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/sizes.h"
#include "shared/types/array-types.h"

static constexpr auto FLUSH_BUFFER_SIZE = (2 * MiB);

U8_max_a flushBuf;

void initLogger(Arena *perm) {
    flushBuf = (U8_max_a){.buf = NEW(perm, U8, FLUSH_BUFFER_SIZE),
                          .cap = FLUSH_BUFFER_SIZE,
                          .len = 0};
}
