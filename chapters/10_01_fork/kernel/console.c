#include "const.h"
#include "console.h"
#include "global.h"
#include "proto.h"
#include "string.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void flush(CONSOLE *con);
PRIVATE void clear_screen(int pos, int len);
PRIVATE void w_copy(unsigned int dst, const unsigned int src, int size);

PUBLIC void init_screen(TTY *tty)
{
    int nr_tty = tty - tty_table;
    tty->console = console_table + nr_tty;

    int v_mem_size = V_MEM_SIZE >> 1;
    int size_per_con = v_mem_size / NR_CONSOLES;
    tty->console->orig = nr_tty * size_per_con;
    tty->console->con_size = size_per_con / SCR_WIDTH * SCR_WIDTH;
    tty->console->cursor = tty->console->crtc_start = tty->console->orig;
    tty->console->is_full = 0;

    tty->console->cursor = tty->console->orig;

    if (nr_tty == 0) {
        tty->console->cursor = disp_pos / 2;
        disp_pos = 0;
    }
    else {
        for (const char *p = "[TTY #?]\n"; *p; p++)
            out_char(tty->console, *p == '?' ? nr_tty + '0' : *p);
    }

    set_cursor(tty->console->cursor);
}

PUBLIC int is_current_console(CONSOLE *con)
{
    return (con == &console_table[nr_current_console]);
}

PUBLIC void out_char(CONSOLE *con, char ch)
{
    u8 *pch = (u8 *)(V_MEM_BASE + con->cursor * 2);
    assert(con->cursor - con->orig < con->con_size);

    int cursor_x = (con->cursor - con->orig) % SCR_WIDTH;
    int cursor_y = (con->cursor - con->orig) / SCR_WIDTH;

    switch (ch) {
        case '\n':
            con->cursor = con->orig + SCR_WIDTH * (cursor_y + 1);
            break;
        case '\b':
            if (con->cursor > con->orig) {
                con->cursor--;
                *(pch - 2) = ' ';
                *(pch - 1) = DEFAULT_CHAR_COLOR;
            }
            break;
        default:
            *pch++ = ch;
            *pch++ = DEFAULT_CHAR_COLOR;
            con->cursor++;
            break;
    }

    if (con->cursor - con->orig >= con->con_size) {
        cursor_x = (con->cursor - con->orig) % SCR_WIDTH;
        cursor_y = (con->cursor - con->orig) / SCR_WIDTH;
        int cp_orig = con->orig + (cursor_y + 1) * SCR_WIDTH - SCR_SIZE;
        w_copy(con->orig, cp_orig, SCR_SIZE - SCR_WIDTH);
        con->crtc_start = con->orig;
        con->cursor = con->orig + (SCR_SIZE - SCR_WIDTH) + cursor_x;
        clear_screen(con->cursor, SCR_WIDTH);
        if (!con->is_full)
            con->is_full = 1;
    }

    assert(con->cursor - con->orig < con->con_size);

    while (con->cursor >= con->crtc_start + SCR_SIZE ||
           con->cursor < con->crtc_start) {
        scroll_screen(con, SCR_UP);

        clear_screen(con->cursor, SCR_WIDTH);
    }

    flush(con);
}

PRIVATE void clear_screen(int pos, int len)
{
    u8 * pch = (u8*)(V_MEM_BASE + pos * 2);
    while (--len >= 0) {
        *pch++ = ' ';
        *pch++ = DEFAULT_CHAR_COLOR;
    }
}

PRIVATE void set_cursor(unsigned int position)
{
    disable_interrupt();
    out_byte(CRTC_ADDR_REG, CURSOR_H);
    out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CURSOR_L);
    out_byte(CRTC_DATA_REG, position & 0xFF);
    enable_interrupt();
}

PRIVATE void set_vedio_start_addr(u32 addr)
{
    disable_int();
    out_byte(CRTC_ADDR_REG, START_ADDR_H);
    out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, START_ADDR_L);
    out_byte(CRTC_DATA_REG, addr & 0xFF);
    enable_int();
}

PUBLIC void select_console(int nr_console)
{
    if ((nr_console < 0) || (nr_console >= NR_CONSOLES))
        return;

    flush(&console_table[nr_current_console = nr_console]);
}

PUBLIC void scroll_screen(CONSOLE *con, int dir)
{
    /*
     * variables below are all in-console-offsets (based on con->orig)
     */
    int oldest; /* addr of the oldest available line in the console */
    int newest; /* .... .. ... latest ......... .... .. ... ....... */
    int scr_top;/* position of the top of current screen */

    newest = (con->cursor - con->orig) / SCR_WIDTH * SCR_WIDTH;
    oldest = con->is_full ? (newest + SCR_WIDTH) % con->con_size : 0;
    scr_top = con->crtc_start - con->orig;

    if (dir == SCR_DN) {
        if (!con->is_full && scr_top > 0) {
            con->crtc_start -= SCR_WIDTH;
        }
        else if (con->is_full && scr_top != oldest) {
            if (con->cursor - con->orig >= con->con_size - SCR_SIZE) {
                if (con->crtc_start != con->orig)
                    con->crtc_start -= SCR_WIDTH;
            }
            else if (con->crtc_start == con->orig) {
                scr_top = con->con_size - SCR_SIZE;
                con->crtc_start = con->orig + scr_top;
            }
            else {
                con->crtc_start -= SCR_WIDTH;
            }
        }
    }
    else if (dir == SCR_UP) {
        if (!con->is_full && newest >= scr_top + SCR_SIZE) {
            con->crtc_start += SCR_WIDTH;
        }
        else if (con->is_full && scr_top + SCR_SIZE - SCR_WIDTH != newest) {
            if (scr_top + SCR_SIZE == con->con_size)
                con->crtc_start = con->orig;
            else
                con->crtc_start += SCR_WIDTH;
        }
    }
    else {
        assert(dir == SCR_DN || dir == SCR_UP);
    }

    flush(con);
}

PRIVATE void flush(CONSOLE *con)
{
    if (is_current_console(con)) {
        set_cursor(con->cursor);
        set_vedio_start_addr(con->crtc_start);
    }
}

PRIVATE void w_copy(unsigned int dst, const unsigned int src, int size)
{
    phys_copy(
        (void*)(V_MEM_BASE + (dst << 1)),
        (void*)(V_MEM_BASE + (src << 1)),
        size << 1
    );
}
