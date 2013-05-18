#ifndef _KERL__CONSOLE_H_
#define _KERL__CONSOLE_H_

#include "const.h"

typedef struct s_console
{
    unsigned int crtc_start; // 当前显示到了什么位置
    unsigned int orig;       // 当前控制台对应显存位置
    unsigned int con_size;   // 当前控制台占的显存大小
    unsigned int cursor;     // 当前光标位置
    int          is_full;
} CONSOLE;

#define DEFAULT_CHAR_COLOR 0x07 // 0000 0111 黑底白字
#define RED_CHAR (MAKE_COLOR(BLUE, RED) | BRIGHT)
#define GRAY_CHAR (MAKE_COLOR(BLACK, BLACK) | BRIGHT)

#define SCR_UP 1
#define SCR_DN -1

#define SCR_SIZE  (80 * 25)
#define SCR_WIDTH  80

#endif
