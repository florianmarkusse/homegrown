#ifndef PLATFORM_ABSTRACTION_MEMORY_MANIPULATION_H
#define PLATFORM_ABSTRACTION_MEMORY_MANIPULATION_H

#ifdef FREESTANDING_ENVIRONMENT
#include "shared/types/types.h"

/* Copy N bytes of SRC to DEST.  */
__attribute((nothrow, nonnull(1, 2))) void *
memcpy(void *__restrict dest, const void *__restrict src, U64 n);
/* Copy N bytes of SRC to DEST, guaranteeing
   correct behavior for overlapping strings.  */
__attribute((nothrow, nonnull(1, 2))) void *memmove(void *dest, const void *src,
                                                    U64 n);

/* Set N bytes of S to C.  */
__attribute((nothrow, nonnull(1))) void *memset(void *s, int c, U64 n);

/* Compare N bytes of S1 and S2.  */
__attribute((nothrow, pure, nonnull(1, 2))) int memcmp(const void *s1,
                                                       const void *s2, U64 n);
#elif POSIX_ENVIRONMENT
void *memcpy(void *dest, const void *src, unsigned long n);
void *memmove(void *dest, const void *src, unsigned long n);
void *memset(void *s, int c, unsigned long n);
int memcmp(const void *dest, const void *src, unsigned long n);
#else
#error "Could not match ENVIRONMENT"
#endif

#endif
