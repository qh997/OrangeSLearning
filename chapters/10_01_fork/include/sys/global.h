#ifndef _KERL__GLOBAL_H_
#define _KERL__GLOBAL_H_

#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
#include "stdio.h"

#ifdef GLOBAL_VARIABLES_HERE
#undef  EXTERN
#define EXTERN
#endif

/* GDT IDT TSS */
EXTERN DESCRIPTOR gdt[GDT_SIZE];
EXTERN u8         gdt_ptr[6];
EXTERN GATE       idt[IDT_SIZE];
EXTERN u8         idt_ptr[6];
EXTERN TSS        tss;

/* 进程 */
EXTERN PROCESS     *p_proc_ready;     // 即将被调度的进程，指向 proc_table 中某项
extern PROCESS     proc_table[];      // 进程表
extern TASK        task_table[];      // 系统任务
extern TASK        user_proc_table[]; // 用户进程
extern char        task_stack[];      // 进程栈

/* 中断 */
EXTERN u32         k_reenter;        // 中断重入标志，0：非重入
extern irq_handler irq_table[];      // 中断向量
extern system_call sys_call_table[]; // 系统调用

/* TTY & Console */
EXTERN int     nr_current_console; // 当前控制台
extern TTY     tty_table[];        // TTY 终端
extern CONSOLE console_table[];    // 控制台

/* FS */
extern u8                 *fsbuf;     // FS 缓冲区
extern const int          FSBUF_SIZE; // 缓冲区大小
EXTERN MESSAGE            fs_msg;
EXTERN struct proc        *pcaller;
EXTERN struct inode       *root_inode;
EXTERN struct super_block super_block[NR_SUPER_BLOCK];
EXTERN struct inode       inode_table[NR_INODE];
EXTERN struct file_desc   f_desc_table[NR_FILE_DESC];

/* MM */
EXTERN MESSAGE mm_msg;
EXTERN int     memory_size;

/* Miscs */
EXTERN int    disp_pos;
EXTERN int    ticks;                // 时钟中断计数
EXTERN int    key_pressed;          // 有键按下
extern struct dev_drv_map dd_map[]; // 设备驱动

#endif
