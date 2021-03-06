#ifndef _KERL__PROC_H_
#define _KERL__PROC_H_

#include "type.h"
#include "protect.h"
#include "const.h"

typedef struct strackframe {
    u32 gs;         // ┓
    u32 fs;         // ┃
    u32 es;         // ┣ save 压栈
    u32 ds;         // ┛
    u32 edi;        // ┓
    u32 esi;        // ┃
    u32 ebp;        // ┃
    u32 kernel_esp; // ┃
    u32 ebx;        // ┣ pushad 压栈
    u32 edx;        // ┃
    u32 ecx;        // ┃
    u32 eax;        // ┛
    u32 retaddr;    // call save 时压栈
    u32 eip;        // ┓
    u32 cs;         // ┃
    u32 eflags;     // ┣ 中断发生时压栈
    u32 esp;        // ┃
    u32 ss;         // ┛
} STACK_FRAME;

typedef struct proc {
    struct strackframe regs;
    u16 ldt_sel;
    DESCRIPTOR ldts[LDT_SIZE];

    int ticks;
    int priority;

    u32 pid;
    char name[16];

    int p_flags;

    MESSAGE *p_msg;
    int p_recvfrom;
    int p_sendto;
    int has_int_msg;
    struct proc *q_sending;
    struct proc *next_sending;

    int p_parent;
    int exit_status;

    struct file_desc *filp[NR_FILES];
} PROCESS;

typedef struct task {
    task_f initial_eip; // 入口地址
    int    stacksize;
    char   name[32];
} TASK;

/* 任务数/进程数 */
#define NR_TASKS         5
#define NR_PROCS        32
#define NR_NATIVE_PROCS  4
#define FIRST_PROC      proc_table[0]
#define LAST_PROC       proc_table[NR_TASKS + NR_PROCS - 1]

/* 进程栈 */
#define STACK_SIZE_DEFAULT  0x4000
#define STACK_SIZE_TTY      STACK_SIZE_DEFAULT
#define STACK_SIZE_SYS      STACK_SIZE_DEFAULT
#define STACK_SIZE_HD       STACK_SIZE_DEFAULT
#define STACK_SIZE_FS       STACK_SIZE_DEFAULT
#define STACK_SIZE_MM       STACK_SIZE_DEFAULT
#define STACK_SIZE_INIT     STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTA    STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTB    STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTC    STACK_SIZE_DEFAULT

#define STACK_SIZE_TOTAL ( \
    STACK_SIZE_TTY + \
    STACK_SIZE_SYS + \
    STACK_SIZE_HD + \
    STACK_SIZE_FS + \
    STACK_SIZE_MM + \
    STACK_SIZE_INIT + \
    STACK_SIZE_TESTA + \
    STACK_SIZE_TESTB + \
    STACK_SIZE_TESTC \
)

#define PROCS_BASE              0xA00000 // 10 MB：应用程序在内存中起始地址
#define PROC_IMAGE_SIZE_DEFAULT 0x100000 //  1 MB：应用程序占内存大小
#define PROC_ORIGIN_STACK       0x400    //  1 KB：参数栈大小（应用程序栈从 0xFFBFF 开始，向下生长）

#define proc2pid(x) (x - proc_table)

#endif
