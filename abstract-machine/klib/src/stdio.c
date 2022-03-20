#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  char buffer[2048];
  va_list arg;
  va_start(arg, fmt);

  int done = vsprintf(buffer, fmt, arg);

  putstr(buffer);

  va_end(arg);
  return done;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, SIZE_MAX, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) { 
  int ret = 0;
  va_list ap;
  va_start(ap, fmt);
  ret = vsprintf(out, fmt, ap);
  va_end(ap);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  int ret = 0;
  va_list ap;

  va_start(ap, fmt);
  ret = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return ret;
}

static const char *HEX_TABLE = "0123456789ABCDEF";

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  size_t fmt_i = 0, out_i = 0;
  bool exit = false;

// self define macro
#define fmt_c (fmt[fmt_i])
#define out_c (out[out_i])
#define append_next(c) do { out[out_i++] = c ; exit = (fmt_c == '\0') || (out_i > n); } while (0);                      
  // end define

  while (!exit) {
    if (fmt_c != '%') {
      // formal condition
      append_next(fmt_c);
      ++fmt_i;
      continue;
    }

    ++fmt_i;
    // format condition
    switch (fmt_c) {
    case 's': {
      char *str_p = va_arg(ap, char *);
      if(str_p == NULL) break;
      for (;*str_p!='\0' && !exit; ++str_p) {
        append_next(*str_p);
      }
      break;
    }
    case 'c': {
      char character = va_arg(ap, int);
      append_next(character);
      break;
    }
    case 'd': {
      char tmp[32]; // numbers' reverse order
      size_t tmp_num = 0;
      int num = va_arg(ap, int);
      // parse number
      if (num < 0) {
        num = -num;
        append_next('-');
      }
      for(; num; num /= 10, ++tmp_num){
        tmp[tmp_num] = HEX_TABLE[num % 10];
      } 
      // format
      for (size_t reverse_num = tmp_num; reverse_num!=0 && !exit;--reverse_num) {
        append_next(tmp[reverse_num-1]);
      }
      break;
    }
    default: 
      exit = true;
      break;
    }
    ++fmt_i;
  }
  if (out_i <= n) {
    append_next('\0');
  }
  return out_i;
}

#endif
