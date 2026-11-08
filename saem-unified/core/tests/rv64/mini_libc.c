/* The five routines the sovereign kernel imports (memcpy, memmove, memset, memcmp, strlen), for freestanding images only. Byte loops on purpose:
 * obviously correct, no alignment assumptions, no tricks. Compiled with -fno-builtin so the compiler cannot turn these loops back into calls to
 * themselves. Test and bring-up tooling: the kernel itself never includes this file. */
#include <stddef.h>

void *memcpy(void *d, const void *s, size_t n) {
  unsigned char *a = d;
  const unsigned char *b = s;
  while (n-- > 0u) *a++ = *b++;
  return d;
}

void *memmove(void *d, const void *s, size_t n) {
  unsigned char *a = d;
  const unsigned char *b = s;
  if (a < b) {
    while (n-- > 0u) *a++ = *b++;
  } else if (a > b) {
    a += n;
    b += n;
    while (n-- > 0u) *--a = *--b;
  }
  return d;
}

void *memset(void *d, int c, size_t n) {
  unsigned char *a = d;
  while (n-- > 0u) *a++ = (unsigned char)c;
  return d;
}

int memcmp(const void *x, const void *y, size_t n) {
  const unsigned char *a = x, *b = y;
  for (; n > 0u; --n, ++a, ++b)
    if (*a != *b) return *a < *b ? -1 : 1;
  return 0;
}

size_t strlen(const char *s) {
  size_t n = 0;
  while (s[n] != '\0') n++;
  return n;
}
