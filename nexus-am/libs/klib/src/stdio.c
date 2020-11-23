#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
/* PA2.2 */
int printf(const char *fmt, ...) {
  va_list args;
  char str[256];
  int length;
  va_start(args,fmt);
  length=vsprintf(str,fmt,args);
  va_end(args);
  for(int i=0;str[i]!='\0';i++){
    _putc(str[i]);
  }
  return length;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return 0;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  int length;
  va_start(args,fmt);
  length=vsprintf(out,fmt,args);
  va_end(args);
  return length;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

#endif
