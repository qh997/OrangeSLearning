#include "sys/const.h"
#include "sys/proto.h"

PUBLIC int read(int fd, void *buf, int count)
{
    MESSAGE msg;

    msg.type = READ;
    msg.FD = fd;
    msg.BUF = (void *)buf;
    msg.CNT = count;

    send_recv(BOTH, TASK_FS, &msg);

    return msg.CNT;
}
