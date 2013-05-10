#include "sys/const.h"
#include "type.h"
#include "sys/proto.h"

PUBLIC int printf(const char *fmt, ...)
{
    int i;
    char buf[256];

    va_list arg = (va_list)((char *)(&fmt) + 4);
    i = vsprintf(buf, fmt, arg);
    buf[i] = 0;
    printx(buf);

    return i;
}
