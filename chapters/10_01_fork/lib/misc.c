#include "const.h"
#include "proto.h"
#include "stdio.h"
#include "string.h"

/*****************************************************************************/
 //* FUNCTION NAME: send_recv
 //*     PRIVILEGE: 1 ~ 3
 //*   RETURN TYPE: int
 //*    PARAMETERS: int function - SEND/RECEIVE/BOTH
 //*                int src_dest - 接受／发送者
 //*                MESSAGE *msg - 消息体
 //*   DESCRIPTION: 系统调用 sendrec 的封装，应避免直接调用 sendrec
/*****************************************************************************/
PUBLIC int send_recv(int function, int src_dest, MESSAGE *msg)
{
    int ret = 0;

    if (function == RECEIVE)
        memset(msg, 0, sizeof(MESSAGE));

    switch (function) {
        case BOTH:
            ret = sendrec(SEND, src_dest, msg);
            if (ret == 0)
                ret = sendrec(RECEIVE, src_dest, msg);
            break;
        case SEND:
        case RECEIVE:
            ret = sendrec(function, src_dest, msg);
            break;
        default:
            assert(function == BOTH ||
                   function == SEND ||
                   function == RECEIVE);
            break;
    }

    return ret;
}

PUBLIC void spin(char *func_name)
{
    printl("\nspinning in %s ...\n", func_name);

    while (1) {}
}

PUBLIC void assertion_failure(char *exp, char *file, char *base_file, int line)
{
    printl("%c  assert(%s) failed: file: %s, base_file: %s, ln%d",
           MAG_CH_ASSERT, exp, file, base_file, line);

    spin("assertion_failure()");

    __asm__ __volatile__("ud2");
}

PUBLIC int memcmp(const void *s1, const void *s2, int n)
{
    if ((s1 == 0) || (s2 == 0))
        return (s1 - s2);

    const char *p1 = (const char *)s1;
    const char *p2 = (const char *)s2;

    for (int i = 0; i < n; i++, p1++, p2++)
        if (*p1 != *p2)
            return (*p1 - *p2);

    return 0;
}

PUBLIC int strcmp(const char * s1, const char *s2)
{
    if ((s1 == 0) || (s2 == 0)) { /* for robustness */
        return (s1 - s2);
    }

    const char * p1 = s1;
    const char * p2 = s2;

    for (; *p1 && *p2; p1++,p2++) {
        if (*p1 != *p2) {
            break;
        }
    }

    return (*p1 - *p2);
}
