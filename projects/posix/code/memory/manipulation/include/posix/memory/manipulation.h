#ifndef POSIX_MEMORY_MANIPULATION_H
#define POSIX_MEMORY_MANIPULATION_H

void *memcpy(void *dest, const void *src, unsigned long n);
void *memmove(void *dest, const void *src, unsigned long n);
void *memset(void *s, int c, unsigned long n);
int memcmp(const void *dest, const void *src, unsigned long n);

#endif
