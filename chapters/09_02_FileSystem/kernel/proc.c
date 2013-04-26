#include "const.h"
#include "proto.h"
#include "global.h"
#include "string.h"

PRIVATE void block(PROCESS *p);
PRIVATE void unblock(PROCESS *p);
PRIVATE int deadlock(int src, int dest);
PRIVATE int msg_send(PROCESS *current, int dest, MESSAGE *m);
PRIVATE int msg_receive(PROCESS *current, int src, MESSAGE *m);

/*****************************************************************************/
//* FUNCTION NAME: schedule
//*     PRIVILEGE: 0
//*   RETURN TYPE: void
//*    PARAMETERS: void
//*   DESCRIPTION: 进程调度
/*****************************************************************************/
PUBLIC void schedule()
{
    PROCESS *p;
    int greatest_ticks = 0;

    while (!greatest_ticks) {
        for (p = &FIRST_PROC; p <= &LAST_PROC; p++)
            if (p->p_flags == 0)
                if (p->ticks > greatest_ticks) {
                    greatest_ticks = p->ticks;
                    p_proc_ready = p;
                }

        if (!greatest_ticks)
            for (p = &FIRST_PROC; p <= &LAST_PROC; p++)
                if (p->p_flags == 0)
                    p->ticks = p->priority;
    }
}

/*****************************************************************************/
//* FUNCTION NAME: ldt_seg_linear
//*     PRIVILEGE: 0 ~ 1
//*   RETURN TYPE: int
//*    PARAMETERS: PROCESS *p - 进程指针
//*                int idx    - LDT 索引号
//*   DESCRIPTION: 获取进程指定 LDT 的基址
/*****************************************************************************/
PUBLIC int ldt_seg_linear(PROCESS *p, int idx)
{
    DESCRIPTOR *d = &p->ldts[idx];

    return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}

/*****************************************************************************/
//* FUNCTION NAME: va2la
//*     PRIVILEGE: 0 ~ 1
//*   RETURN TYPE: void *
//*    PARAMETERS: int pid
//*                void *va
//*   DESCRIPTION: 获取 va 的线性地址（基址 + 偏移）
/*****************************************************************************/
PUBLIC void *va2la(int pid, void *va)
{
    PROCESS *p = &proc_table[pid];
    u32 seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
    u32 la = seg_base + (u32)va;

    if (pid < NR_TASKS + NR_PROCS)
        assert(la == (u32)va);

    return (void *)la;
}

/*****************************************************************************/
//* FUNCTION NAME: sys_sendrec
//*     PRIVILEGE: 0
//*   RETURN TYPE: int
//*    PARAMETERS: PROCESS *p   - 调用者进程
//*                int function - SEND/RECEIVE
//*                int src_dest - 接受／发送者
//*                MESSAGE *m   - 消息体
//*   DESCRIPTION: 系统调用 sendrec 的核心处理程序
/*****************************************************************************/
PUBLIC int sys_sendrec(PROCESS *p, int function, int src_dest, MESSAGE *m)
{
    assert(k_reenter == 0);
    assert((src_dest >= 0 && src_dest < NR_TASKS + NR_PROCS) ||
           src_dest == ANY ||
           src_dest == INTERRUPT);

    int ret = 0;
    int caller = proc2pid(p);
    MESSAGE *mla = (MESSAGE *)va2la(caller, m);
    mla->source = caller;

    assert(mla->source != src_dest);

    if (function == SEND) {
        ret = msg_send(p, src_dest, m);
    }
    else if (function == RECEIVE) {
        ret = msg_receive(p, src_dest, m);
    }
    else {
        panic("{sys_sendrec} invalid function: "
              "%d (SEND:%d, RECEIVE:%d).", function, SEND, RECEIVE);
    }

    return ret;
}

/*****************************************************************************/
//* FUNCTION NAME: send_recv
//*     PRIVILEGE: 1 ~ 3
//*   RETURN TYPE: int
//*    PARAMETERS: int function - SEND/RECEIVE/BOTH
//*                int src_dest - 接受／发送者
//*                MESSAGE *msg - 消息体
//*   DESCRIPTION: 系统调用 sendrec 的封装，应避免直接调用 sendrec
/*****************************************************************************/
PUBLIC int send_recv(int function, int src_dest, MESSAGE *msg)
{
    int ret = 0;

    if (function == RECEIVE)
        memset(msg, 0, sizeof(MESSAGE));

    switch (function) {
        case BOTH:
            ret = sendrec(SEND, src_dest, msg);
            if (ret == 0)
                ret = sendrec(RECEIVE, src_dest, msg);
            break;
        case SEND:
        case RECEIVE:
            ret = sendrec(function, src_dest, msg);
            break;
        default:
            assert(function == BOTH ||
                   function == SEND ||
                   function == RECEIVE);
            break;
    }

    return ret;
}

/*****************************************************************************/
//* FUNCTION NAME: reset_msg
//*     PRIVILEGE: 0 ~ 3
//*   RETURN TYPE: void
//*    PARAMETERS: MESSAGE *m
//*   DESCRIPTION: 清空消息体
/*****************************************************************************/
PUBLIC void reset_msg(MESSAGE *m)
{
    memset(m, 0, sizeof(MESSAGE));
}

/*****************************************************************************/
//* FUNCTION NAME: dump_msg
//*     PRIVILEGE: 0 ~ 3
//*   RETURN TYPE: void
//*    PARAMETERS: char *title
//*                MESSAGE *m
//*   DESCRIPTION: 
/*****************************************************************************/
PUBLIC void dump_msg(const char *title, MESSAGE *m)
{
    //int packed = 0;
    printl("{%s}<0x%x>{%ssrc:%s(%d),%stype:%d,%s(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n}\n",
           title,
           (int)m,
           //packed ? " " : "\n        ",
           "\n        ",
           proc_table[m->source].name,
           m->source,
           //packed ? " " : "\n        ",
           "\n        ",
           m->type,
           //packed ? " " : "\n        ",
           "\n        ",
           m->u.m3.m3i1,
           m->u.m3.m3i2,
           m->u.m3.m3i3,
           m->u.m3.m3i4,
           (int)m->u.m3.m3p1,
           (int)m->u.m3.m3p2
        );
}

/*****************************************************************************/
//* FUNCTION NAME: inform_int
//*     PRIVILEGE: 0
//*   RETURN TYPE: void
//*    PARAMETERS: int task_nr
//*   DESCRIPTION: 通知 task_nr 进程有一个中断发生
/*****************************************************************************/
PUBLIC void inform_int(int task_nr)
{
    PROCESS *p = proc_table + task_nr;

    if ((p->p_flags & RECEIVING) &&
        ((p->p_recvfrom == INTERRUPT) || (p->p_recvfrom == ANY))) {
        p->p_msg->source = INTERRUPT;
        p->p_msg->type = HARD_INT;
        p->p_msg = 0;
        p->has_int_msg = 0;
        p->p_flags &= ~RECEIVING;
        p->p_recvfrom = NO_TASK;
        assert(p->p_flags == 0);
        unblock(p);

        assert(p->p_flags == 0);
        assert(p->p_msg == 0);
        assert(p->p_recvfrom == NO_TASK);
        assert(p->p_sendto == NO_TASK);
    }
    else {
        p->has_int_msg = 1;
    }
}

/*****************************************************************************/
//* FUNCTION NAME: block
//*     PRIVILEGE: 0
//*   RETURN TYPE: void
//*    PARAMETERS: PROCESS *p
//*   DESCRIPTION: 阻塞进程
/*****************************************************************************/
PRIVATE void block(PROCESS *p)
{
    assert(p->p_flags);
    schedule();
}

/*****************************************************************************/
//* FUNCTION NAME: unblock
//*     PRIVILEGE: 0
//*   RETURN TYPE: void
//*    PARAMETERS: PROCESS *p
//*   DESCRIPTION: 恢复进程
/*****************************************************************************/
PRIVATE void unblock(PROCESS *p)
{
    assert(p->p_flags == 0);
}

/*****************************************************************************/
//* FUNCTION NAME: deadlock
//*     PRIVILEGE: 0
//*   RETURN TYPE: int - 成功 0；失败 1
//*    PARAMETERS: int src
//*                int dest
//*   DESCRIPTION: 判断消息发送是否发生死锁
/*****************************************************************************/
PRIVATE int deadlock(int src, int dest)
{
    PROCESS *p = proc_table + dest;
    while (TRUE) {
        if (p->p_flags & SENDING) {
            if (p->p_sendto == src) {
                p = proc_table + dest;
                printl("=_=%s", p->name);
                do {
                    assert(p->p_msg);
                    p = proc_table + p->p_sendto;
                    printl("->%s", p->name);
                } while (p != proc_table + src);
                printl("=_=");

                return 1;
            }

            p = proc_table + p->p_sendto;
        }
        else
            break;
    }

    return 0;
}

/*****************************************************************************/
//* FUNCTION NAME: msg_send
//*     PRIVILEGE: 0
//*   RETURN TYPE: int
//*    PARAMETERS: PROCESS *current
//*                int dest
//*                MESSAGE *m
//*   DESCRIPTION: 发送一个消息给 dest 进程
/*****************************************************************************/
PRIVATE int msg_send(PROCESS *current, int dest, MESSAGE *m)
{
    PROCESS *sender = current;
    PROCESS *p_dest = proc_table + dest;

    assert(proc2pid(sender) != dest); // 不能自己给自己发消息

    if (deadlock(proc2pid(sender), dest))
        panic(">>DEADLOCK<< %s->%s", sender->name, p_dest->name);

    if ((p_dest->p_flags & RECEIVING) &&           // 如果目的进程正在等待接受消息
        (p_dest->p_recvfrom == proc2pid(sender) || // 并且指定发送者为本进程
         p_dest->p_recvfrom == ANY)) {             // 或任何进程
        assert(p_dest->p_msg);
        assert(m);

        phys_copy(va2la(dest, p_dest->p_msg),
                  va2la(proc2pid(sender), m),
                  sizeof(MESSAGE));
        p_dest->p_msg = 0;
        p_dest->p_flags &= ~RECEIVING;
        p_dest->p_recvfrom = NO_TASK;
        unblock(p_dest);

        assert(p_dest->p_flags == 0);
        assert(p_dest->p_msg == 0);
        assert(p_dest->p_recvfrom == NO_TASK);
        assert(p_dest->p_sendto == NO_TASK);
        assert(sender->p_flags == 0);
        assert(sender->p_msg == 0);
        assert(sender->p_recvfrom == NO_TASK);
        assert(sender->p_sendto == NO_TASK);
    }
    else { // 如果目的进程没有等待接受本进程的消息
        sender->p_flags |= SENDING;
        assert(sender->p_flags == SENDING);
        sender->p_sendto = dest;
        sender->p_msg = m;

        /* 将自己添加到目的进程的发送者队列中去 */
        PROCESS *p;
        if (p_dest->q_sending) {
            p = p_dest->q_sending;
            while (p->next_sending)
                p = p->next_sending;
            p->next_sending = sender;
        }
        else {
            p_dest->q_sending = sender;
        }
        sender->next_sending = NULL;

        block(sender);

        assert(sender->p_flags == SENDING);
        assert(sender->p_msg != 0);
        assert(sender->p_recvfrom == NO_TASK);
        assert(sender->p_sendto == dest);
    }

    return 0;
}

/*****************************************************************************/
//* FUNCTION NAME: msg_receive
//*     PRIVILEGE: 0
//*   RETURN TYPE: int
//*    PARAMETERS: PROCESS *current
//*                int src
//*                MESSAGE *m
//*   DESCRIPTION: 接收一个来自 src 进程的消息
/*****************************************************************************/
PRIVATE int msg_receive(PROCESS *current, int src, MESSAGE *m)
{
    PROCESS *p_who_wanna_recv = current;
    PROCESS *p_from = 0;
    PROCESS *prev = 0;
    int copyok = 0;

    assert(proc2pid(p_who_wanna_recv) != src); // 不能自己接收自己的消息

    if ((p_who_wanna_recv->has_int_msg) &&
        ((src == ANY) || (src == INTERRUPT))) {
        MESSAGE msg;
        reset_msg(&msg);
        msg.source = INTERRUPT;
        msg.type = HARD_INT;
        assert(m);
        phys_copy(va2la(proc2pid(p_who_wanna_recv), m),
                  &msg, sizeof(MESSAGE));

        p_who_wanna_recv->has_int_msg = FALSE;

        assert(p_who_wanna_recv->p_flags == 0);
        assert(p_who_wanna_recv->p_msg == 0);
        assert(p_who_wanna_recv->p_sendto == NO_TASK);
        assert(p_who_wanna_recv->q_sending == 0);

        return 0;
    }

    if (src == ANY) { // 如果要等待来自任何进程的消息
        if (p_who_wanna_recv->q_sending) { // 如果消息队列不为空
            p_from = p_who_wanna_recv->q_sending; // 取消息队列中第一个
            copyok = TRUE;

            assert(p_who_wanna_recv->p_flags == 0);
            assert(p_who_wanna_recv->p_msg == 0);
            assert(p_who_wanna_recv->p_recvfrom == NO_TASK);
            assert(p_who_wanna_recv->p_sendto == NO_TASK);
            assert(p_who_wanna_recv->q_sending != 0);
            assert(p_from->p_flags == SENDING);
            assert(p_from->p_msg != 0);
            assert(p_from->p_recvfrom == NO_TASK);
            assert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
        }
    }
    else { // 如果要等待指定进程的消息
        p_from = &proc_table[src];

        if ((p_from->p_flags & SENDING) &&                      // 如果指定的进程正在发送消息
            (p_from->p_sendto == proc2pid(p_who_wanna_recv))) { // 并且就是发送给该进程
            copyok = TRUE;

            PROCESS *p = p_who_wanna_recv->q_sending;
            assert(p);

            /* 在消息队列中找到指定的进程 */
            while (p) {
                assert(p_from->p_flags & SENDING);
                if (proc2pid(p) == src) {
                    p_from = p;
                    break;
                }

                prev = p;
                p = p->next_sending;
            }

            assert(p_who_wanna_recv->p_flags == 0);
            assert(p_who_wanna_recv->p_msg == 0);
            assert(p_who_wanna_recv->p_recvfrom == NO_TASK);
            assert(p_who_wanna_recv->p_sendto == NO_TASK);
            assert(p_who_wanna_recv->q_sending != 0);
            assert(p_from->p_flags == SENDING);
            assert(p_from->p_msg != 0);
            assert(p_from->p_recvfrom == NO_TASK);
            assert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
        }
    }

    if (copyok) { // 如果成功接收到了消息
        if (p_from == p_who_wanna_recv->q_sending) {
            assert(prev == 0);
            p_who_wanna_recv->q_sending = p_from->next_sending;
            p_from->next_sending = 0;
        }
        else {
            assert(prev);
            prev->next_sending = p_from->next_sending;
            p_from->next_sending = 0;
        }

        assert(m);
        assert(p_from->p_msg);
        
        phys_copy(va2la(proc2pid(p_who_wanna_recv), m),
                  va2la(proc2pid(p_from), p_from->p_msg),
                  sizeof(MESSAGE));

        p_from->p_msg = 0;
        p_from->p_sendto = NO_TASK;
        p_from->p_flags &= ~SENDING;
        unblock(p_from); // 恢复发送者
    }
    else {
        p_who_wanna_recv->p_flags |= RECEIVING;
        p_who_wanna_recv->p_msg = m;

        p_who_wanna_recv->p_recvfrom = src == ANY 
                                     ? ANY : proc2pid(p_from);

        block(p_who_wanna_recv); // 挂起

        assert(p_who_wanna_recv->p_flags == RECEIVING);
        assert(p_who_wanna_recv->p_msg != 0);
        assert(p_who_wanna_recv->p_recvfrom != NO_TASK);
        assert(p_who_wanna_recv->p_sendto == NO_TASK);
        assert(p_who_wanna_recv->q_sending == 0);
    }

    return 0;
}
