#include "const.h"
#include "proc.h"
#include "proto.h"
#include "stdio.h"

PUBLIC int fork()
{
    MESSAGE msg;
    msg.type = FORK;

    send_recv(BOTH, TASK_MM, &msg);
    assert(SYSCALL_RET == msg.type);
    assert(0 == msg.RETVAL);

    return msg.PID;
}
