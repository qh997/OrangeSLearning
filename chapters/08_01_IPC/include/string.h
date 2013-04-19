#ifndef _KERL__STRING_H_
#define _KERL__STRING_H_

#include "const.h"

PUBLIC void *memcpy(void *p_dst, void *pSrc, int size);
PUBLIC void memset(void *p_dst, char ch, int size);
PUBLIC char* strcpy(char* p_dst, char* p_src);
PUBLIC int strlen(char* p_str);

#define phys_copy memcpy
#define phys_set memset

#endif
