#ifndef SHARED_MEMORY_MANIPULATION_MANIPULATION_H
#define SHARED_MEMORY_MANIPULATION_MANIPULATION_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef FREESTANDING_BUILD

#include "interoperation/types.h"

/* Copy N bytes of SRC to DEST.  */
__attribute((nothrow, nonnull(1, 2))) void *
memcpy(void *__restrict dest, const void *__restrict src, I64 n);
/* Copy N bytes of SRC to DEST, guaranteeing
   correct behavior for overlapping strings.  */
__attribute((nothrow, nonnull(1, 2))) void *memmove(void *dest, const void *src,
                                                    I64 n);

/* Set N bytes of S to C.  */
__attribute((nothrow, nonnull(1))) void *memset(void *s, int c, I64 n);

/* Compare N bytes of S1 and S2.  */
__attribute((nothrow, pure, nonnull(1, 2))) int memcmp(const void *s1,
                                                       const void *s2, I64 n);

#else

// Not sure how to make these work with the platform you're building on. I guess
// you will get a warning/error if the types are wrong.
void *memcpy(void *dest, const void *src, unsigned long n);
void *memmove(void *dest, const void *src, unsigned long n);
void *memset(void *s, int c, unsigned long n);
int memcmp(const void *dest, const void *src, unsigned long n);

#endif

#ifdef __cplusplus
}
#endif
#endif
