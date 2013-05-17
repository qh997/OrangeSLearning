#include "const.h"
#include "proc.h"
#include "proto.h"
#include "stdio.h"

PUBLIC void exit(int status)
{
    MESSAGE msg;
    msg.type = EXIT;
    msg.STATUS = status;

    send_recv(BOTH, TASK_MM, &msg);
    assert(msg.type == SYSCALL_RET); // 不应该运行到这里
}
