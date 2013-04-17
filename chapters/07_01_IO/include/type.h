#ifndef _KERL__TYPE_H_
#define _KERL__TYPE_H_

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef char *va_list;

typedef void (*int_handler)();
typedef void (*task_f)();
typedef void (*irq_handler)(int irq);
typedef void (*system_call);

#endif