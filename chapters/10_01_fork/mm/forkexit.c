#include "const.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include "protect.h"
#include "string.h"

PUBLIC int do_fork()
{
    struct proc *p = proc_table;
    int i;
    for (i = 0; i < NR_TASKS + NR_PROCS; i++, p++)
        if (p->p_flags == FREE_SLOT)
            break;

    int child_pid = i;
    assert(p == &proc_table[child_pid]);
    assert(child_pid >= NR_TASKS + NR_NATIVE_PROCS);
    if (i == NR_TASKS + NR_PROCS)
        return -1;
    assert(i < NR_TASKS + NR_PROCS);

    int pid = mm_msg.source;
    u16 child_ldt_sel = p->ldt_sel;
    *p = proc_table[pid];
    p->ldt_sel = child_ldt_sel;
    p->p_parent = pid;
    sprintf(p->name, "%s_%d", proc_table[pid].name, child_pid);

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

    int child_base = alloc_mem(child_pid, caller_T_size);
    printl("{MM} 0x%x <- 0x%x (0x%x bytes)\n",
           child_base, caller_T_base, caller_T_size);

    phys_copy((void *)child_base, (void *)caller_T_base, caller_T_size);

    init_desc(&p->ldts[INDEX_LDT_C],
          child_base,
          (PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
          DA_LIMIT_4K | DA_32 | DA_C | PRIVILEGE_USER << 5);
    init_desc(&p->ldts[INDEX_LDT_RW],
          child_base,
          (PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
          DA_LIMIT_4K | DA_32 | DA_DRW | PRIVILEGE_USER << 5);

    MESSAGE msg2fs;
    msg2fs.type = FORK;
    msg2fs.PID = child_pid;
    send_recv(BOTH, TASK_FS, &msg2fs);

    mm_msg.PID = child_pid;

    MESSAGE m;
    m.type = SYSCALL_RET;
    m.RETVAL = 0;
    m.PID = 0;
    send_recv(SEND, child_pid, &m);

    return 0;
}

PUBLIC void do_exit(int status)
{
}

PUBLIC void do_wait()
{
}
