#include "const.h"
#include "type.h"
#include "proto.h"
#include "stdio.h"

PUBLIC int printf(const char *fmt, ...)
{
    int i;
    char buf[STR_DEFAULT_LEN];

    va_list arg = (va_list)((char *)(&fmt) + 4);
    i = vsprintf(buf, fmt, arg);
    int c = write(1, buf, i); // 默认使用该进程的第二个文件描述符
    
    assert(c == i);

    return i;
}

PUBLIC int printl(const char *fmt, ...)
{
    int i;
    char buf[STR_DEFAULT_LEN];

    va_list arg = (va_list)((char *)(&fmt) + 4);
    i = vsprintf(buf, fmt, arg);
    printx(buf);

    return i;
}
