#include "const.h"
#include "proto.h"
#include "keyboard.h"

PUBLIC void task_tty()
{
    while (1)
    {
        keyboard_read();
    }
}

PUBLIC void in_process(u32 key)
{
    char output[2] = {'\0', '\0'};

    if (!(key & FLAG_EXT))
    {
        output[0] = key & 0xFF;
        disp_str(output);
    }
}
