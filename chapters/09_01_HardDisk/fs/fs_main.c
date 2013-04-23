#include "const.h"
#include "proto.h"

PUBLIC void task_fs()
{
    printl("Task FS begins.\n");

    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    send_recv(BOTH, TASK_HD, &driver_msg);

    spin("FS");
}
