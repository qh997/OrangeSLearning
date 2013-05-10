#include "sys/const.h"
#include "sys/proto.h"

PUBLIC int write(int fd, const void *buf, int count)
{
    MESSAGE msg;
    msg.type = WRITE;
    msg.FD = fd;
    msg.BUF = (void *)buf;
    msg.CNT = count;

    send_recv(BOTH, TASK_FS, &msg);

    return msg.CNT;
}
