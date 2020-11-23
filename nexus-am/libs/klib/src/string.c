#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
/* PA2.2 */
size_t strlen(const char *s) {
  if (s == NULL) return 0;
  size_t len = 0;
  while ((*s++)!='\0') len++;
  return len;
}

char *strcpy(char* dst,const char* src) {
  if(dst == NULL || src == NULL) return dst;
  char *t = dst;
  while ((*t++ = *src++) !='\0');
  return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
  if(dst == NULL || src == NULL) return dst;
  char *t = dst;
  size_t i = 0;
  for (i=0; i<n&&(*src != '\0'); i++){
    *t++ = *src++;
  }
  return dst;
}

char* strcat(char* dst, const char* src) {
  if(dst == NULL || src == NULL) return dst;
  char *t = dst;
  while (*t++!='\0');
  while ((*t++ = *src++) !='\0');
  return dst;
}

int strcmp(const char* s1, const char* s2) {
  assert(s1 != NULL && s2 != NULL);
  while(*s1 != '\0' && *s1 == *s2 ) {
		s1 ++;
		s2 ++;
	}
	
	int ret = *(unsigned char *)s1 - *(unsigned char *)s2;
	
	if(ret > 0) ret = 1;
	else if(ret < 0) ret = -1;
	
	return  ret;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  assert(s1 != NULL && s2 != NULL);
	int ret = 0;
	while(n -- ) {
		ret = *s1  - *s2;
		if(ret != 0 || *s1 == '\0') break;
		s1 ++;
		s2 ++;
	}
	if(ret > 0) ret = 1;
	else if(ret < 0) ret = -1;
	return ret;
}

void* memset(void* v,int c,size_t n) {
  if (v == NULL) return NULL;
  char *t = (char *)v;
  while(n -- ) *t ++ = c; 
  return v;
}

void* memcpy(void* out, const void* in, size_t n) {
  char *src = (char *)in;
	char *dst = (char *)out;
	assert(dst <= src || dst >= src + n );	
	while(n --) *dst ++ = *src ++;
	return out;
}

int memcmp(const void* s1, const void* s2, size_t n){
  assert(s1 != NULL && s2 != NULL);
	unsigned char *v1 = (unsigned char *)s1;
	unsigned char *v2 = (unsigned char *)s2;
	int ret = 0;
	while(n -- ) {
		ret = *v1  - *v2;
		if(ret != 0) break;
		v1 ++;
		v2 ++;
	}
	if(ret > 0) ret = 1;
	else if(ret < 0) ret = -1;
	return ret;
}

#endif
