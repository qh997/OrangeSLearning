#include "const.h"
#include "proto.h"
#include "global.h"
#include "string.h"

PRIVATE int msg_send(PROCESS *current, int dest, MESSAGE *m);
PRIVATE int msg_receive(PROCESS *current, int src, MESSAGE *m);

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

PUBLIC int ldt_seg_linear(PROCESS *p, int idx)
{
    DESCRIPTOR *d = &p->ldts[idx];

    return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}

PUBLIC void *va2la(int pid, void *va)
{
    PROCESS *p = &proc_table[pid];
    u32 seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
    u32 la = seg_base + (u32)va;

    if (pid < NR_TASKS + NR_PROCS)
        assert(la == (u32)va);

    return (void *)la;
}

PUBLIC int sys_get_ticks()
{
    return ticks;
}

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

PUBLIC void reset_msg(MESSAGE *p)
{
    memset(p, 0, sizeof(MESSAGE));
}

PRIVATE void block(PROCESS *p)
{
    assert(p->p_flags);
    schedule();
}

PRIVATE void unblock(PROCESS *p)
{
    assert(p->p_flags == 0);
}

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

PRIVATE int msg_send(PROCESS *current, int dest, MESSAGE *m)
{
    PROCESS *sender = current;
    PROCESS *p_dest = proc_table + dest;

    assert(proc2pid(sender) != dest); // 不能自己给自己发消息

    if (deadlock(proc2pid(sender), dest))
        panic(">>DEADLOCK<< %s->%s", sender->name, p_dest->name);

    if ((p_dest->p_flags & RECEIVING) &&           // 如果目的进程正在等待接受消息
        (p_dest->p_recvfrom == proc2pid(sender) || // 并且发送者为本进程
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

PRIVATE int msg_receive(PROCESS *current, int src, MESSAGE *m)
{
    PROCESS *p_who_wanna_recv = current;
    PROCESS *p_from = 0;
    PROCESS *prev = 0;
    int copyok = 0;

    assert(proc2pid(p_who_wanna_recv) != src); // 不能自己接收自己的消息

    if (p_who_wanna_recv->has_int_msg &&
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

    if (src == ANY) {
        if (p_who_wanna_recv->q_sending) {
            p_from = p_who_wanna_recv->q_sending;
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
    else {
        p_from = &proc_table[src];

        if ((p_from->p_flags & SENDING) &&
            (p_from->p_sendto == proc2pid(p_who_wanna_recv))) {
            copyok = TRUE;

            PROCESS *p = p_who_wanna_recv->q_sending;
            assert(p);

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

    if (copyok) {
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
        unblock(p_from);
    }
    else {
        p_who_wanna_recv->p_flags |= RECEIVING;
        p_who_wanna_recv->p_msg = m;

        p_who_wanna_recv->p_recvfrom = src == ANY 
                                     ? ANY : proc2pid(p_from);

        block(p_who_wanna_recv);

        assert(p_who_wanna_recv->p_flags == RECEIVING);
        assert(p_who_wanna_recv->p_msg != 0);
        assert(p_who_wanna_recv->p_recvfrom != NO_TASK);
        assert(p_who_wanna_recv->p_sendto == NO_TASK);
        assert(p_who_wanna_recv->q_sending == 0);
    }

    return 0;
}
