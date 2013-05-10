#include "sys/const.h"
#include "type.h"
#include "sys/proto.h"
#include "sys/global.h"

PUBLIC void task_sys()
{
    MESSAGE msg;

    while (1) {
        send_recv(RECEIVE, ANY, &msg);      // 等待接受消息
        int src = msg.source;               // 消息来源
        switch (msg.type) {
            case GET_TICKS:
                msg.RETVAL = ticks;
                send_recv(SEND, src, &msg); // 发送消息
                break;

            default:
                panic("unknow msg type");
                break;
        }
    }
}
