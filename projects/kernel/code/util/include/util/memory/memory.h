#ifndef UTIL_MEMORY_MEMORY_H
#define UTIL_MEMORY_MEMORY_H
#ifdef __cplusplus
extern "C" {
#endif

#include "util/types.h"

/* Copy N bytes of SRC to DEST.  */
__attribute((nothrow, nonnull(1, 2))) void *
memcpy(void *__restrict dest, const void *__restrict src, int64_t n);
/* Copy N bytes of SRC to DEST, guaranteeing
   correct behavior for overlapping strings.  */
__attribute((nothrow, nonnull(1, 2))) void *memmove(void *dest, const void *src,
                                                    int64_t n);

/* Set N bytes of S to C.  */
__attribute((nothrow, nonnull(1))) void *memset(void *s, int c, int64_t n);

/* Compare N bytes of S1 and S2.  */
__attribute((nothrow, pure, nonnull(1, 2))) int
memcmp(const void *s1, const void *s2, int64_t n);

#ifdef __cplusplus
}
#endif
#endif
