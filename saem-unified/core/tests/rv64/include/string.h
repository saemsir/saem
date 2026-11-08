/* A freestanding target has no <string.h>. This declares exactly the five routines the sovereign kernel may import; core/tests/rv64/mini_libc.c
 * implements them for bare-metal test images. The kernel sources are unchanged: they include <string.h> as on a hosted system. */
#ifndef COS_FREESTANDING_STRING_H
#define COS_FREESTANDING_STRING_H
#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
size_t strlen(const char *s);

#endif
