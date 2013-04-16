#ifndef _KERL__TTY_H_
#define _KERL__TTY_H_

#include "const.h"
#include "console.h"

#define TTY_IN_BYTES 256

struct s_console;

typedef struct s_tty
{
    u32 in_buf[TTY_IN_BYTES];
    u32 *p_inbuf_head;
    u32 *p_inbuf_tail;
    int inbuf_count;

    struct s_console * p_console;
} TTY;

#endif
