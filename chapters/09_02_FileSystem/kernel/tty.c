#include "const.h"
#include "proto.h"
#include "keyboard.h"
#include "global.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "string.h"

#define TTY_FIRST (tty_table)
#define TTY_END   (tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY *tty);
PRIVATE void tty_dev_read(TTY *tty);
PRIVATE void tty_dev_write(TTY *tty);
PRIVATE void tty_do_read(TTY *tty, MESSAGE *msg);
PRIVATE void tty_do_write(TTY *tty, MESSAGE *msg);
PRIVATE void put_key(TTY *tty, u32 key);

PUBLIC void task_tty()
{
    TTY *tty;
    MESSAGE msg;

    init_keyboard();

    for (tty = TTY_FIRST; tty < TTY_END; tty++)
        init_tty(tty);

    select_console(0);

    while (1) {
        for (tty = TTY_FIRST; tty < TTY_END; tty++) {
            tty_dev_read(tty);
            tty_dev_write(tty);
        }

        send_recv(RECEIVE, ANY, &msg);

        int src = msg.source;
        assert(src != TASK_TTY);

        TTY *ptty = &tty_table[msg.DEVICE];

        switch (msg.type) {
            case DEV_OPEN:
                reset_msg(&msg);
                msg.type = SYSCALL_RET;
                send_recv(SEND, src, &msg);
                break;

            case DEV_READ:
                tty_do_read(ptty, &msg);
                break;

            case DEV_WRITE:
                tty_do_write(ptty, &msg);
                break;

            case HARD_INT:
                key_pressed = 0;
                continue; // 回到循环开始处

            default:
                dump_msg("TTY::unknown msg", &msg);
                break;
        }
    }
}

PUBLIC void in_process(TTY *tty, u32 key)
{
    if (!(key & FLAG_EXT)) {
        put_key(tty, key);
    }
    else {
        int raw_code = key & MASK_RAW;
        switch (raw_code)
        {
            case ENTER:
                put_key(tty, '\n');
                break;
            case BACKSPACE:
                put_key(tty, '\b');
                break;
            case UP:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R))
                    scroll_screen(tty->console, SCR_UP);
                break;
            case DOWN:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R))
                    scroll_screen(tty->console, SCR_DN);
                break;
            case F1: 
            case F2:
            case F3:
            case F4:
            case F5:
            case F6:
            case F7:
            case F8:
            case F9:
            case F10:
            case F11:
            case F12:
                if ((key & (FLAG_ALT_L | FLAG_SHIFT_L)) ||
                    (key & (FLAG_ALT_R | FLAG_SHIFT_R)))
                    select_console(raw_code - F1);
            default:
                break;
        }
    }
}

PUBLIC int sys_printx(PROCESS *p_proc, char *s)
{
    const char *p;
    char ch;
    char reenter_err[] = "? k_reenter is incorrect for unknown reason";
    reenter_err[0] = MAG_CH_PANIC;

    if (k_reenter == 0)
        p = va2la(proc2pid(p_proc), s);
    else if (k_reenter > 0)
        p = s;
    else
        p = reenter_err;

    if ((*p == MAG_CH_PANIC) ||
        (*p == MAG_CH_ASSERT && p_proc_ready < &proc_table[NR_TASKS])) {
        disable_interrupt();
        char *v = (char *)V_MEM_BASE;
        const char *q = p + 1;

        while (v < (char *)(V_MEM_BASE + V_MEM_SIZE)) {
            *v++ = *q++;
            *v++ = RED_CHAR;
            if (!*q) {
                while (((int)v - V_MEM_BASE) % (SCREEN_WIDTH * 16)) {
                    v++;
                    *v++ = GRAY_CHAR;
                }
                q = p + 1;
            }
        }

        __asm__ __volatile__("hlt");
    }

    while ((ch = *p++) != 0) {
        if (ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT)
            continue;

        out_char(TTY_FIRST->console, ch);
    }

    return 0;
}

PRIVATE void init_tty(TTY *tty)
{
    tty->ibuf_cnt = 0;
    tty->ibuf_head = tty->ibuf_tail = tty->ibuf;

    init_screen(tty);
}

PRIVATE void tty_dev_read(TTY *tty)
{
    if (is_current_console(tty->console))
        keyboard_read(tty);
}

PRIVATE void tty_dev_write(TTY *tty)
{
    while (tty->ibuf_cnt) {
        char ch = *(tty->ibuf_tail);
        tty->ibuf_tail++;
        if (tty->ibuf_tail == tty->ibuf + TTY_IN_BYTES)
            tty->ibuf_tail = tty->ibuf;
        tty->ibuf_cnt--;

        if (tty->tty_left_cnt) {
            if (ch >= ' ' && ch <= '~') { // printable
                out_char(tty->console, ch);
                void *p = tty->tty_req_buf
                        + tty->tty_trans_cnt;
                phys_copy(p, (void *)va2la(TASK_TTY, &ch), 1);
                tty->tty_trans_cnt++;
                tty->tty_left_cnt--;
            }
            /* 只删除输入的字符 */
            else if (ch == '\b' && tty->tty_trans_cnt) {
                out_char(tty->console, ch);
                tty->tty_trans_cnt--;
                tty->tty_left_cnt++;
            }

            /* 遇到回车或字符数目达到预设，恢复自己（对应于 tty_do_read） */
            if (ch == '\n' || tty->tty_left_cnt == 0) {
                out_char(tty->console, '\n');
                MESSAGE msg;
                msg.type = RESUME_PROC;
                msg.PROC_NR = tty->tty_procnr;
                msg.CNT = tty->tty_trans_cnt;
                send_recv(SEND, tty->tty_caller, &msg);
                tty->tty_left_cnt = 0;
            }
        }
    }
}

PRIVATE void tty_do_read(TTY *tty, MESSAGE *msg)
{
    /* tell the tty: */
    tty->tty_caller = msg->source; // who called, usually FS
    tty->tty_procnr = msg->PROC_NR; // who wants the chars, like TestB
    tty->tty_req_buf = va2la(tty->tty_procnr, msg->BUF); // where the chars should be put
    tty->tty_left_cnt = msg->CNT; // how many chars are requested
    tty->tty_trans_cnt= 0; // how many chars have been transferred

    /* 通知发送进程将自己挂起 */
    msg->type = SUSPEND_PROC;
    msg->CNT = tty->tty_left_cnt;
    send_recv(SEND, tty->tty_caller, msg);
}

PRIVATE void tty_do_write(TTY *tty, MESSAGE *msg)
{
    char buf[TTY_OUT_BUF_LEN];
    char *p = (char *)va2la(msg->PROC_NR, msg->BUF);
    int i = msg->CNT;

    while (i) {
        int bytes = min(TTY_OUT_BUF_LEN, i);
        phys_copy(va2la(TASK_TTY, buf), (void *)p, bytes);
        for (int j = 0; j < bytes; j++)
            out_char(tty->console, buf[j]);
        i -= bytes;
        p += bytes;
    }

    msg->type = SYSCALL_RET;
    send_recv(SEND, msg->source, msg);
}

PRIVATE void put_key(TTY *tty, u32 key)
{
    if (tty->ibuf_cnt < TTY_IN_BYTES) {
        *(tty->ibuf_head) = key;
        tty->ibuf_head++;
        if (tty->ibuf_head == tty->ibuf + TTY_IN_BYTES)
            tty->ibuf_head = tty->ibuf;
        tty->ibuf_cnt++;
    }
}
