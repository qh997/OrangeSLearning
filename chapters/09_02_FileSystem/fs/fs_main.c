#include "const.h"
#include "proto.h"
#include "global.h"

PUBLIC void task_fs()
{
    printl("Task FS begins.\n");

    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    spin("FS");
}
