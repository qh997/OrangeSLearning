#include "const.h"
#include "console.h"
#include "global.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);

PUBLIC int is_current_console(CONSOLE *p_con)
{
    return (p_con == &console_table[nr_current_console]);
}

PUBLIC void out_char(CONSOLE *p_con, char ch)
{
    u8 *p_vmem = (u8 *)(V_MEM_BASE + disp_pos);

    *p_vmem++ = ch;
    *p_vmem++ = DEFAULT_CHAR_COLOR;
    disp_pos += 2;

    set_cursor(disp_pos / 2);
}

PRIVATE void set_cursor(unsigned int position)
{
    disable_interrupt();
    out_byte(CRTC_ADDR_REG, CURSOR_H);
    out_byte(CRTC_DATA_REG, ((disp_pos / 2) >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CURSOR_L);
    out_byte(CRTC_DATA_REG, (disp_pos / 2) & 0xFF);
    enable_interrupt();
}
