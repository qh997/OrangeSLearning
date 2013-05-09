#include "const.h"
#include "type.h"
#include "string.h"
#include "proto.h"
#include "stdio.h"

PUBLIC int open(const char *pathname, int flags)
{
    MESSAGE msg;

    msg.type = OPEN;
    msg.PATHNAME = (void *)pathname;
    msg.FLAGS = flags;
    msg.NAME_LEN = strlen(pathname);

    send_recv(BOTH, TASK_FS, &msg);
    assert(msg.type == SYSCALL_RET);

    return msg.FD;
}
