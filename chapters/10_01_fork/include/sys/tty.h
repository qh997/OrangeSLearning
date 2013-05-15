#ifndef _KERL__TTY_H_
#define _KERL__TTY_H_

#include "const.h"
#include "console.h"

#define TTY_IN_BYTES 256
#define TTY_OUT_BUF_LEN 2

typedef struct s_tty
{
    u32 ibuf[TTY_IN_BYTES]; // TTY 输入缓冲区
    u32 *ibuf_head;         // 指向缓冲区中下一个空闲位置
    u32 *ibuf_tail;         // 指向键盘任务应处理的键值
    int ibuf_cnt;           // 缓冲区中已经填充了多少

    int  tty_caller;        // TTY 的调用进程，例如 FS
    int  tty_procnr;        // 接收字符的进程，例如 TestB
    void *tty_req_buf;      // 缓冲
    int  tty_left_cnt;      // 剩余字符数目
    int  tty_trans_cnt;     // 已经传输字符数目

    struct s_console *console;
} TTY;

#endif
