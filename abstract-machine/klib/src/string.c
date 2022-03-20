#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

typedef unsigned int op_t;
#define OPSIZ 4
void *memset(void *s, int c, size_t n) {
  // shim
  void *dstpp = s;
  size_t len = n;

  long dstp = (long)dstpp;
  if (len >= 8)
    {
      size_t xlen;
      op_t cccc;
      cccc = (unsigned char) c;
      cccc |= cccc << 8;
      cccc |= cccc << 16;
      if (OPSIZ > 4)
        /* Do the shift in two steps to avoid warning if long has 32 bits.  */
        cccc |= (cccc << 16) << 16;
      /* There are at least some bytes to set.
         No need to test for LEN == 0 in this alignment loop.  */
      while (dstp % OPSIZ != 0)
        {
          ((unsigned char *) dstp)[0] = c;
          dstp += 1;
          len -= 1;
        }
      /* Write 8 `op_t' per iteration until less than 8 `op_t' remain.  */
      xlen = len / (OPSIZ * 8);
      while (xlen > 0)
        {
          ((op_t *) dstp)[0] = cccc;
          ((op_t *) dstp)[1] = cccc;
          ((op_t *) dstp)[2] = cccc;
          ((op_t *) dstp)[3] = cccc;
          ((op_t *) dstp)[4] = cccc;
          ((op_t *) dstp)[5] = cccc;
          ((op_t *) dstp)[6] = cccc;
          ((op_t *) dstp)[7] = cccc;
          dstp += 8 * OPSIZ;
          xlen -= 1;
        }
      len %= OPSIZ * 8;
      /* Write 1 `op_t' per iteration until less than OPSIZ bytes remain.  */
      xlen = len / OPSIZ;
      while (xlen > 0)
        {
          ((op_t *) dstp)[0] = cccc;
          dstp += OPSIZ;
          xlen -= 1;
        }
      len %= OPSIZ;
    }
  /* Write the last few bytes.  */
  while (len > 0)
    {
      ((unsigned char *) dstp)[0] = c;
      dstp += 1;
      len -= 1;
    }
  return dstpp;
}

void *memcpy(void *out, const void *in, size_t n) {
  char *dstp = (char*) out;
  char *srcp = (char*) in;
  unsigned i;
  for (i = 0; i < n; ++i)
    dstp[i] = srcp[i];
  return out;
}

static inline void move_byte_forward(uint8_t *dest_m, const uint8_t *src_m,
                                     size_t count) {
  for (size_t offset = 0; count; --count, ++offset)
    dest_m[offset] = src_m[offset];
}

static inline void move_byte_backward(uint8_t *dest_m, const uint8_t *src_m,
                                      size_t count) {
  for (size_t offset = count - 1; count; --count, --offset)
    dest_m[offset] = src_m[offset];
}

// fork llvm-libc
void *memmove(void *dest, const void *src, size_t n) {
  uint8_t *dest_temp = dest;
  const uint8_t *src_temp = src;
  assert(dest && src); ;// not null

  // If the distance between src_c and dest_c is equal to or greater
  // than count (integerAbs(src_c - dest_c) >= count), they would not overlap.
  // e.g.   greater     equal       overlapping
  //        [12345678]  [12345678]  [12345678]
  // src_c: [_ab_____]  [_ab_____]  [_ab_____]
  // dest_c:[_____yz_]  [___yz___]  [__yz____]
  if (abs((char *)dest - (char *)src)>= n)
    return memcpy(dest, src, n);
  
  // Overlap cases.
  // If dest_c starts before src_c (dest_c < src_c), copy forward(pointer add 1)
  // from beginning to end.
  // If dest_c starts after src_c (dest_c > src_c), copy backward(pointer add
  // -1) from end to beginning.
  // If dest_c and src_c start at the same address (dest_c == src_c),
  // just return dest.
  // e.g.    forward      backward
  //             *-->        <--*
  // src_c : [___abcde_]  [_abcde___]
  // dest_c: [_abc--___]  [___--cde_]
  if (dest_temp < src_temp)
    move_byte_forward(dest_temp, src_temp, n);
  if (dest_temp > src_temp)
    move_byte_backward(dest_temp, src_temp, n);
  return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  uint8_t *p1 = (uint8_t *)s1;
  uint8_t *p2 = (uint8_t *)s2;
  /**
   * p1 and p2 are the same memory? easy peasy! bail out
   */
  if (p1 == p2) {
    return 0;
  }

  if (p1 == NULL) {
    return 1;
  }

  if (p2 == NULL) {
    return -1;
  }

  // This for loop does the comparing and pointer moving...
  while (n && (*p1++ == *p2++)) {
    --n;
  }

  // if i == length, then we have passed the test
  return (n == 0) ? 0 : (*(uint8_t *)p1 - *(uint8_t *)p2);
}

size_t strlen(const char *s) {
  size_t count = 0;
  while (*s) { ++count; ++s; }
  return count;
}

char *strcpy(char *dst, const char *src) {
  size_t len = strlen(src);
  memcpy(dst, src, len+1);
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  memcpy(dst, src, n);
  return dst;
}

char *strcat(char *dst, const char *src) {
  strcpy(dst+strlen(dst), src);
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  unsigned char c1, c2;
  do
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0')
        return c1 - c2;
    }
  while (c1 == c2);
  return c1 - c2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  unsigned char c1, c2;
  do
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0')
        return c1 - c2;
    }
  while (--n && (c1 == c2));
  return c1 - c2;
}



#endif
