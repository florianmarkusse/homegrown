#ifndef MEMORY_MANIPULATION_H
#define MEMORY_MANIPULATION_H

// Not sure how to make these work with the platform you're building on. I guess
// you will get a warning/error if the types are wrong.
void *memcpy(void *dest, const void *src, unsigned long n);
void *memmove(void *dest, const void *src, unsigned long n);
void *memset(void *s, int c, unsigned long n);
int memcmp(const void *dest, const void *src, unsigned long n);

#endif
