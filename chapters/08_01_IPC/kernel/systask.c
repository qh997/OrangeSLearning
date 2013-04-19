#include "const.h"
#include "type.h"
#include "proto.h"
#include "global.h"

PUBLIC void task_sys()
{
    MESSAGE msg;

    while (1) {
        send_recv(RECEIVE, ANY, &msg);
        int src = msg.source;
        switch (msg.type) {
            case GET_TICKS:
                msg.RETVAL = ticks;
                send_recv(SEND, src, &msg);
                break;
            default:
                panic("unknow msg type");
                break;
        }
    }
}
