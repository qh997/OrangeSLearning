#include "const.h"
#include "type.h"
#include "proto.h"
#include "string.h"

PUBLIC int vsprintf(char *buf, const char *fmt, va_list args)
{
    char *p;
    char tmp[256];
    int m;
    va_list p_next_arg = args;
    
    for (p = buf; *fmt; fmt++) {
        if (*fmt != '%') {
            *p++ = *fmt;
            continue;
        }

        fmt++;

        switch (*fmt) {
            case 'x':
            case 'd':
                itoa(tmp, *((int *)p_next_arg));
                strcpy(p, tmp);
                p_next_arg += 4;
                p += strlen(tmp);
                break;
            case 's':
                strcpy(p, *(char **)p_next_arg);
                p += strlen(*((char **)p_next_arg));
                p_next_arg += 4;
                break;
            case 'c':
                *p++ = *(char *)p_next_arg;
                p_next_arg += 4;
                break;
            default:
                break;
        }
    }

    return (p - buf);
}
