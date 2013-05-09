#ifndef _KERL__STRING_H_
#define _KERL__STRING_H_

#include "const.h"

PUBLIC void *memcpy(void *p_dst, const void *pSrc, const int size);
PUBLIC void memset(void *p_dst, const char ch, const int size);
PUBLIC char *strcpy(char *p_dst, const char *p_src);
PUBLIC int strlen(const char *p_str);

PUBLIC int memcmp(const void *s1, const void *s2, int n);

#define phys_copy memcpy
#define phys_set memset

#endif
