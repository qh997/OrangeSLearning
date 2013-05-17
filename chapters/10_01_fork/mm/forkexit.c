#include "const.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include "protect.h"
#include "string.h"

PRIVATE void cleanup(struct proc *proc);

/*****************************************************************************/
 //* FUNCTION NAME: do_fork
 //*     PRIVILEGE: 1
 //*   RETURN TYPE: int
 //*    PARAMETERS: void
 //*   DESCRIPTION: 
/*****************************************************************************/
PUBLIC int do_fork()
{
    /********************/
    /* alloc proc_table */
    /********************/
    struct proc *p = proc_table;
    int i;
    for (i = 0; i < NR_TASKS + NR_PROCS; i++, p++)
        if (p->p_flags == FREE_SLOT)
            break;
    int child_pid = i;
    assert(p == &proc_table[child_pid]);
    assert(child_pid >= NR_TASKS + NR_NATIVE_PROCS);
    if (i == NR_TASKS + NR_PROCS)
        return -1; // 进程表满了
    assert(i < NR_TASKS + NR_PROCS);

    /************************/
    /* duplicate proc_table */
    /************************/
    int pid = mm_msg.source;
    u16 child_ldt_sel = p->ldt_sel;
    *p = proc_table[pid];
    p->ldt_sel = child_ldt_sel;
    p->p_parent = pid;
    sprintf(p->name, "%s_%d", proc_table[pid].name, child_pid);

    /*********************************/
    /* duplicate text/data/stack seg */
    /*********************************/
    struct descriptor *ppd;
    ppd = &proc_table[pid].ldts[INDEX_LDT_C];
    int caller_T_base = reassembly(ppd->base_high, 24, ppd->base_mid, 16, ppd->base_low);
    int caller_T_limit = reassembly(0, 0, (ppd->limit_high_attr2 & 0xF), 16, ppd->limit_low);
    int caller_T_size = ((caller_T_limit + 1) * ((ppd->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ? 4096 : 1));

    ppd = &proc_table[pid].ldts[INDEX_LDT_RW];
    int caller_D_S_base = reassembly(ppd->base_high, 24, ppd->base_mid, 16, ppd->base_low);
    int caller_D_S_limit = reassembly(0, 0, (ppd->limit_high_attr2 & 0xF), 16, ppd->limit_low);
    int caller_D_S_size = ((caller_T_limit + 1) * ((ppd->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ? 4096 : 1));

    assert((caller_T_base  == caller_D_S_base ) &&
           (caller_T_limit == caller_D_S_limit) &&
           (caller_T_size  == caller_D_S_size ));

    /****************/
    /* alloc memory */
    /****************/
    /* base of child proc, T, D & S segments share the same space,
     * so we allocate memory just once
     */
    int child_base = alloc_mem(child_pid, caller_T_size);
    printl("{MM} 0x%x <- 0x%x (0x%x bytes)\n", child_base, caller_T_base, caller_T_size);

    /* 复制父进程内存空间 */
    phys_copy((void *)child_base, (void *)caller_T_base, caller_T_size);

    /* LDT */
    init_desc(
        &p->ldts[INDEX_LDT_C],
        child_base,
        (PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
        DA_LIMIT_4K | DA_32 | DA_C | PRIVILEGE_USER << 5
    );
    init_desc(
        &p->ldts[INDEX_LDT_RW],
        child_base,
        (PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
        DA_LIMIT_4K | DA_32 | DA_DRW | PRIVILEGE_USER << 5
    );

    /***********/
    /* tell FS */
    /***********/
    MESSAGE msg2fs;
    msg2fs.type = FORK;
    msg2fs.PID = child_pid;
    send_recv(BOTH, TASK_FS, &msg2fs);

    /**********/
    /* finish */
    /**********/
    /* 返回给父进程的子进程 PID */
    mm_msg.PID = child_pid;

    /* 子进程从此开始 */
    MESSAGE m;
    m.type = SYSCALL_RET;
    m.RETVAL = 0;
    m.PID = 0;
    send_recv(SEND, child_pid, &m);
    /* 此时，父进程和子进程都处于 RECEIVE 状态
     * 且 PC 都指向 syscall.asm::sendrec 的 ret
     * send_recv(SEND, child_pid, &m) 之后
     * 子进程将开始从 PC 处执行
     */

    return 0;
}

/*****************************************************************************/
 //* FUNCTION NAME: do_exit
 //*     PRIVILEGE: 1
 //*   RETURN TYPE: void
 //*    PARAMETERS: int status
 //*   DESCRIPTION: 
/*****************************************************************************/
PUBLIC void do_exit(int status)
{
    int pid = mm_msg.source;
    int parent_pid = proc_table[pid].p_parent;
    struct proc *p = &proc_table[pid];

    /* 通知 FS */
    MESSAGE msg2fs;
    msg2fs.type = EXIT;
    msg2fs.PID = pid;
    send_recv(BOTH, TASK_FS, &msg2fs);

    free_mem(pid);

    p->exit_status = status;

    if (proc_table[parent_pid].p_flags & WAITING) {
        /* 如果父进程正在 WAITING，则解除其 WAITING 状态 
         * 并通知父进程，然后彻底清除本进程
         */
        proc_table[parent_pid].p_flags &= ~WAITING;
        cleanup(p);
    }
    else {
        p->p_flags |= HANGING;
    }

    for (int i = 0; i < NR_TASKS + NR_PROCS; i++) {
        if (proc_table[i].p_parent == pid) {
            /* 如果当前进程有子进程就将这些子进程的父进程设为 INIT */
            proc_table[i].p_parent = INIT;
            if ((proc_table[INIT].p_flags & WAITING) &&
                (proc_table[i].p_flags & HANGING)) {
                /* 如果 INIT 进程正在 WAITING，则解除其 WAITING 状态 
                 * 并通知 INIT 进程，然后彻底清除子进程
                 */
                proc_table[INIT].p_flags &= ~WAITING;
                cleanup(&proc_table[i]);
            }
        }
    }
}

/*****************************************************************************/
 //* FUNCTION NAME: do_wait
 //*     PRIVILEGE: 1
 //*   RETURN TYPE: void
 //*    PARAMETERS: void
 //*   DESCRIPTION: 
/*****************************************************************************/
PUBLIC void do_wait()
{
    int pid = mm_msg.source;
    int children = 0;

    struct proc *p_proc = proc_table;
    for (int i = 0; i < NR_TASKS + NR_PROCS; i++, p_proc++) {
        if (p_proc->p_parent == pid) {
            children++;
            if (p_proc->p_flags & HANGING) {
                /* 如果有子进程正在 HANGING
                 * 则彻底清除该子进程
                 */
                cleanup(p_proc);
                return;
            }
        }
    }

    if (children) { // 存在子进程，但没有子进程处于 HANGING
        proc_table[pid].p_flags |= WAITING;
    }
    else { // 没有子进程
        MESSAGE msg;
        msg.type = SYSCALL_RET;
        msg.PID = NO_TASK;
        send_recv(SEND, pid, &msg);
    }
}

/*****************************************************************************/
 //* FUNCTION NAME: cleanup
 //*     PRIVILEGE: 1
 //*   RETURN TYPE: void
 //*    PARAMETERS: struct proc *proc
 //*   DESCRIPTION: 
/*****************************************************************************/
PRIVATE void cleanup(struct proc *proc)
{
    MESSAGE msg2parent;
    msg2parent.type = SYSCALL_RET;
    msg2parent.PID = proc2pid(proc);
    msg2parent.STATUS = proc->exit_status;
    send_recv(SEND, proc->p_parent, &msg2parent);

    proc->p_flags = FREE_SLOT;
}
