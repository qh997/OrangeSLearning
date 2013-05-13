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

    PROCESS *p_proc = proc_table;
    TASK *p_task = task_table;
    char *p_task_stack = task_stack + STACK_SIZE_TOTAL;
    u16 selector_ldt = SELECTOR_LDT_FIRST;

    /* 初始化进程表 */
    for (int i = 0; i < NR_TASKS + NR_PROCS; i++) {
        u8 privilege, rpl;
        int eflags, prio;
        if (i < NR_TASKS) { // 系统任务
            p_task = task_table + i;
            privilege = PRIVILEGE_TASK;
            rpl = RPL_TASK;
            eflags = 0x1202;
            prio = 15;
        }
        else { // 用户进程
            p_task = user_proc_table + (i - NR_TASKS);
            privilege = PRIVILEGE_USER;
            rpl = RPL_USER;
            eflags = 0x202;
            prio = 5;
        }

        strcpy(p_proc->name, p_task->name);
        p_proc->pid = i;
        p_proc->ldt_sel = selector_ldt;

        memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
        p_proc->ldts[0].attr1 = DA_C | privilege << 5;
        memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
        p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;

        /* 代码段指向第一个 IDT */
        p_proc->regs.cs = ((0 * 8) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;

        /* 其他段指向第二个 IDT */
        p_proc->regs.ds = ((1 * 8) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.es = ((1 * 8) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.fs = ((1 * 8) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.ss = ((1 * 8) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;

        /* gs 不变 */
        p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

        p_proc->regs.eip= (u32)p_task->initial_eip;
        p_proc->regs.esp= (u32)p_task_stack;
        p_proc->regs.eflags = eflags;

        //p_proc->nr_tty = 0;

        p_proc->p_flags = 0;
        p_proc->p_msg = 0;
        p_proc->p_recvfrom = NO_TASK;
        p_proc->p_sendto = NO_TASK;
        p_proc->has_int_msg = 0;
        p_proc->q_sending = 0;
        p_proc->next_sending = 0;

        p_proc->priority = p_proc->ticks = prio;

        p_task_stack -= p_task->stacksize;
        p_proc++;
        p_task++;
        selector_ldt += 1 << 3; // selector_ldt += 8
    }

    //proc_table[NR_TASKS + 0].nr_tty = 1;
    //proc_table[NR_TASKS + 1].nr_tty = 2;
    //proc_table[NR_TASKS + 2].nr_tty = 2;

    k_reenter = 0;
    ticks = 0;

    p_proc_ready = proc_table;

    init_clock();

    restart();

    while(1){}
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

/**************************/
/*     USER PROCESSES     */
/*                        */
/*   run in privilege 3   */
/**************************/

void TestA()
{
    int fd;
    int i, n;

    char filename[MAX_FILENAME_LEN+1] = "blah";
    const char bufw[] = "abcde";
    const int rd_bytes = 3;
    char bufr[rd_bytes];

    assert(rd_bytes <= strlen(bufw));

    /* create */
    fd = open(filename, O_CREAT | O_RDWR);
    assert(fd != -1);
    printl("File created: %s (fd %d)\n", filename, fd);

    /* write */
    n = write(fd, bufw, strlen(bufw));
    assert(n == strlen(bufw));

    /* close */
    close(fd);

    /* open */
    fd = open(filename, O_RDWR);
    assert(fd != -1);
    printl("File opened. fd: %d\n", fd);

    /* read */
    n = read(fd, bufr, rd_bytes);
    assert(n == rd_bytes);
    bufr[n] = 0;
    printl("%d bytes read: %s\n", n, bufr);

    /* close */
    close(fd);

    char * filenames[] = {"/foo", "/bar", "/baz"};

    /* create files */
    for (i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++) {
        fd = open(filenames[i], O_CREAT | O_RDWR);
        assert(fd != -1);
        printl("File created: %s (fd %d)\n", filenames[i], fd);
        close(fd);
    }

    char * rfilenames[] = {"/bar", "/foo", "/baz", "/dev_tty0"};

    /* remove files */
    for (i = 0; i < sizeof(rfilenames) / sizeof(rfilenames[0]); i++) {
        if (unlink(rfilenames[i]) == 0)
            printl("File removed: %s\n", rfilenames[i]);
        else
            printl("Failed to remove file: %s\n", rfilenames[i]);
    }

    spin("TestA");
}

void TestB()
{
    char tty_name[] = "/dev_tty1";

    int fd_stdin  = open(tty_name, O_RDWR);
    assert(fd_stdin  == 0);
    int fd_stdout = open(tty_name, O_RDWR);
    assert(fd_stdout == 1);

    char rdbuf[128];

    while (1) {
        printf("$ ");
        int r = read(fd_stdin, rdbuf, 70);
        rdbuf[r] = 0;

        if (strcmp(rdbuf, "hello") == 0)
            printf("hello world!\n");
        else
            if (rdbuf[0])
                printf("{%s}\n", rdbuf);
    }

    assert(0); /* never arrive here */
}

void TestC()
{
    milli_delay(5000);
    while(1) {
        printl("C");
        milli_delay(1000);
    }
}
