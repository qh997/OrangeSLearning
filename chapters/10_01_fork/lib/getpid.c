#include "const.h"
#include "proc.h"
#include "proto.h"
#include "stdio.h"

PUBLIC int getpid()
{
    MESSAGE msg;
    msg.type = GET_PID;

    send_recv(BOTH, TASK_SYS, &msg);
    assert(SYSCALL_RET == msg.type);

    return msg.PID;
}
