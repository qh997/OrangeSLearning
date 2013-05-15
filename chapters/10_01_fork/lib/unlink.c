#include "const.h"
#include "string.h"
#include "proto.h"

PUBLIC int unlink(const char *pathname)
{
    MESSAGE msg;
    msg.type = UNLINK;

    msg.PATHNAME = (void *)pathname;
    msg.NAME_LEN = strlen(pathname);

    send_recv(BOTH, TASK_FS, &msg);

    return msg.RETVAL;
}
