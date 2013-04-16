#ifndef _KERL__CONSOLE_H_
#define _KERL__CONSOLE_H_

#include "const.h"

#define DEFAULT_CHAR_COLOR 0x07

typedef struct s_console
{
    unsigned int current_start_addr;
    unsigned int original_addr;
    unsigned int v_mem_limit;
    unsigned int cursor;
} CONSOLE;

PUBLIC int is_current_console(CONSOLE *p_con);
PUBLIC void out_char(CONSOLE *p_con, char ch);

#endif
