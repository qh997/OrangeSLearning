#include "const.h"
#include "proto.h"
#include "proc.h"
#include "global.h"
#include "protect.h"
#include "string.h"
#include "stdio.h"

/*****************************************************************************/
 //* FUNCTION NAME: kernel_main
 //*     PRIVILEGE: 0
 //*   RETURN TYPE: int
 //*    PARAMETERS: void
 //*   DESCRIPTION: 
/*****************************************************************************/
PUBLIC int kernel_main()
{
    disp_str("-----\"kernel_main\" begins-----\n");

    PROCESS *p = proc_table;
    TASK *t = 0;
    char *stk = task_stack + STACK_SIZE_TOTAL;

    /* 初始化进程表 */
    for (int i = 0;
         i < NR_TASKS + NR_PROCS;
         i++, p++, t++) {
        /* 子进程预留 */
        if (i >= NR_TASKS + NR_NATIVE_PROCS) {
            p->p_flags = FREE_SLOT;
            continue;
        }

        u8 priv, rpl;
        int eflags, prio;
        if (i < NR_TASKS) { // 系统任务
            t = task_table + i;
            priv = PRIVILEGE_TASK;
            rpl = RPL_TASK;
            eflags = 0x1202;
            prio = 15;
        }
        else { // 用户进程
            t = user_proc_table + (i - NR_TASKS);
            priv = PRIVILEGE_USER;
            rpl = RPL_USER;
            eflags = 0x202;
            prio = 5;
        }

        strcpy(p->name, t->name);
        p->p_parent = NO_TASK;

        if (strcmp(t->name, "INIT") != 0) { // 不是 INIT 进程
            /* 直接使用 0～4G 内存空间 */
            p->ldts[INDEX_LDT_C] = gdt[SELECTOR_KERNEL_CS >> 3];
            p->ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DS >> 3];

            p->ldts[INDEX_LDT_C].attr1 = DA_C | priv << 5;
            p->ldts[INDEX_LDT_RW].attr1 = DA_DRW | priv << 5;
        }
        else { // INIT 进程
            unsigned int k_base, k_limit;
            assert(0 == get_kernel_map(&k_base, &k_limit));

            /* 初始化 LDT */
            init_desc(
                &p->ldts[INDEX_LDT_C],
                0, 
                (k_base + k_limit) >> LIMIT_4K_SHIFT,
                DA_32 | DA_LIMIT_4K | DA_C | priv << 5
            );

            init_desc(
                &p->ldts[INDEX_LDT_RW],
                0,
                (k_base + k_limit) >> LIMIT_4K_SHIFT,
                DA_32 | DA_LIMIT_4K | DA_DRW | priv << 5
            );
        }

        /* 代码段指向第一个 LDT */
        p->regs.cs = INDEX_LDT_C << 3 | SA_TIL | rpl;

        /* 其他段指向第二个 LDT */
        p->regs.ds =
        p->regs.es =
        p->regs.fs =
        p->regs.ss = INDEX_LDT_RW << 3 | SA_TIL | rpl;

        /* gs 不变 */
        p->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

        p->regs.eip= (u32)t->initial_eip; // 入口地址
        p->regs.esp= (u32)stk;            // 栈指针
        p->regs.eflags = eflags;

        /* IPC */
        p->p_flags = 0;
        p->p_msg = 0;
        p->p_recvfrom = NO_TASK;
        p->p_sendto = NO_TASK;
        p->has_int_msg = 0;
        p->q_sending = 0;
        p->next_sending = 0;

        for (int j = 0; j < NR_FILES; j++)
            p->filp[j] = 0;

        /* 优先级 */
        p->priority = p->ticks = prio;
        
        stk -= t->stacksize;
    }

    k_reenter = 0;
    ticks = 0;

    p_proc_ready = proc_table;

    init_clock();

    restart();

    while (1);
}

/*****************************************************************************/
 //* FUNCTION NAME: get_ticks
 //*     PRIVILEGE: 3
 //*   RETURN TYPE: int
 //*    PARAMETERS: void
 //*   DESCRIPTION: 
/*****************************************************************************/
PUBLIC int get_ticks()
{
    MESSAGE msg;
    reset_msg(&msg);
    msg.type = GET_TICKS;
    send_recv(BOTH, TASK_SYS, &msg);
    return msg.RETVAL;
}

/*****************************************************************************/
 //* FUNCTION NAME: panic
 //*     PRIVILEGE: 3
 //*   RETURN TYPE: int
 //*    PARAMETERS: const char *fmt
 //*                ...
 //*   DESCRIPTION: 
/*****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
    char buf[256];

    va_list arg = (va_list)((char *)&fmt + 4);
    vsprintf(buf, fmt, arg);

    printl("%c ||panic!! %s", MAG_CH_PANIC, buf);

    __asm__ __volatile__("ud2");
}

/******************************************/
/****          USER PROCESSES          ****/
/****                                  ****/
/****        run in privilege 3        ****/
/******************************************/

void Init()
{
    int fd_stdin = open("/dev_tty0", O_RDWR);
    assert(0 == fd_stdin);
    int fd_stdout = open("/dev_tty0", O_RDWR);
    assert(1 == fd_stdout);

    printf("Init() is running ... %d\n", p_proc_ready->pid);

    int pid = fork();
    if (pid != 0) {
        printf("parent is running, child pid: %d\n", pid);
        int s;
        int child = wait(&s);
        printf("child (%d) exited with status: %d.\n", child, s);
    }
    else {
        printf("child is running, pid: %d\n", getpid());
        exit(123);
    }

    while (1) {
        int s;
        int child = wait(&s);
        printf("child (%d) exited with status: %d.\n", child, s);
    }
}

void TestA()
{
    while (1);
}

void TestB()
{
    while (1);
}

void TestC()
{
    while (1);
}
