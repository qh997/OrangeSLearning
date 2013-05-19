#include "const.h"
#include "stdio.h"
#include "string.h"
#include "proto.h"

PUBLIC int stat(const char *path, struct stat *buf)
{
    MESSAGE msg;

    msg.type = STAT;
    msg.PATHNAME = (void *)path;
    msg.BUF = (void *)buf;
    msg.NAME_LEN = strlen(path);

    send_recv(BOTH, TASK_FS, &msg);
    assert(msg.type == SYSCALL_RET);

    return msg.RETVAL;
}
