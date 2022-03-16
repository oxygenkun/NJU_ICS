#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

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

void *memmove(void *dst, const void *src, size_t n) {
  //解决内存重叠的问题，就需要加上逆序拷贝，适用于任何类型
  unsigned char *dest_tmp = (unsigned char *)dst;
  const unsigned char *src_tmp = (const unsigned char *)src;
  assert(dst && src);
  if (src_tmp > dest_tmp || src_tmp + n <= dest_tmp) //情况1和情况2
  {
    while (n--) //正序复制
    {
      *dest_tmp++ = *src_tmp++;
    }
  } else //情况3，逆序赋值
  {
    //逆序打印，需要把指针调整位置
    dest_tmp += n - 1;
    src_tmp += n - 1;
    while (n--) {
      *(dest_tmp--) = *(src_tmp--);
    }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  char *dstp = (char*) out;
  char *srcp = (char*) in;
  unsigned i;
  for (i = 0; i < n; ++i)
    dstp[i] = srcp[i];
  return out;
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

#endif
