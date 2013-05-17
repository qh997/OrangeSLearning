#include "const.h"
#include "proc.h"
#include "proto.h"

PUBLIC int wait(int *status)
{
    MESSAGE msg;
    msg.type = WAIT;

    send_recv(BOTH, TASK_MM, &msg);
    *status = msg.STATUS;

    return (msg.PID == NO_TASK ? -1 : msg.PID);
}
