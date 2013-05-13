#ifndef _KERL__PROTO_H_
#define _KERL__PROTO_H_

#include "const.h"
#include "type.h"
#include "tty.h"
#include "proc.h"
#include "fs.h"

/* kliba.asm */
PUBLIC void disp_str(char *info);
PUBLIC void disp_color_str(char *info, int color);
PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8 in_byte(u16 port);
PUBLIC int disable_irq(int irq);
PUBLIC void enable_irq(int irq);
PUBLIC void disable_int();
PUBLIC void enable_int();
PUBLIC void port_read(u16 port, void *buf, int n);
PUBLIC void port_write(u16 port, void *buf, int n);

/* klib.c */
PUBLIC char *itoa(char *str, int num);
PUBLIC void disp_int(int input);
PUBLIC void delay(int time);

/* protect.c */
PUBLIC void init_prot();
PUBLIC void exception_handler(int vec_no, int err_code, int eip, int cs, int eflags);
PUBLIC u32 seg2phys(u16 seg);

/* i8259.c*/
PUBLIC void init_8259A();
PUBLIC void put_irq_handler(int irq, irq_handler handler);

/* kernel.asm */
void restart();

/* kmain.c */
PUBLIC int kernel_main();
PUBLIC int get_ticks();
void TestA();
void TestB();
void TestC();
PUBLIC void panic(const char *fmt, ...);

/* clock.c */
PUBLIC void milli_delay(int milli_sec);
PUBLIC void init_clock();

/* hd.c */
PUBLIC void task_hd();

/* fs/main.c */
PUBLIC void task_fs();
PUBLIC int rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_nr, void *buf);
PUBLIC struct super_block *get_super_block(int dev);
PUBLIC struct inode *get_inode(int dev, int num);
PUBLIC void put_inode(struct inode *pinode);
PUBLIC void sync_inode(struct inode * p);

/* fs/open.c */
PUBLIC int do_open();
PUBLIC int do_close();

/* fs/read_write.c */
PUBLIC int do_rdwt();

/* fs/link.c */
PUBLIC int do_unlink();

/* fs/misc.c */
PUBLIC int search_file(char *path);
PUBLIC int strip_path(char *filename, const char *pathname, struct inode **ppinode);

/* proc.c */
PUBLIC void schedule();
PUBLIC int ldt_seg_linear(PROCESS *p, int idx);
PUBLIC void *va2la(int pid, void *va);
PUBLIC int sys_sendrec();
PUBLIC int send_recv(int function, int src_dest, MESSAGE *msg);
PUBLIC void reset_msg(MESSAGE *p);
PUBLIC void dump_msg(const char *title, MESSAGE *m);
PUBLIC void inform_int(int task_nr);

/* systask.c */
PUBLIC void task_sys();

/* syscall.asm */
PUBLIC int printx(char *str);
PUBLIC int sendrec(int function, int src_dest, MESSAGE* p_msg);

/* keyboard.c */
PUBLIC void init_keyboard();
PUBLIC void keyboard_read();

/* tty.c */
PUBLIC void task_tty();
PUBLIC void in_process(TTY *p_tty, u32 key);
PUBLIC int sys_printx(PROCESS *p_proc, char *s);

/* console.c */
PUBLIC void init_screen(TTY *p_tty);
PUBLIC int is_current_console(CONSOLE *p_con);
PUBLIC void out_char(CONSOLE *p_con, char ch);
PUBLIC void select_console(int nr_console);
PUBLIC void scroll_screen(CONSOLE *p_con, int direction);

/* vsprintf.c */
PUBLIC int vsprintf(char *buf, const char *fmt, va_list args);
PUBLIC int sprintf(char *buf, const char *fmt, ...);

/* printf.c */
PUBLIC int printf(const char *fmt, ...);
PUBLIC int printl(const char *fmt, ...);

/* misc.c */
PUBLIC void spin(char *func_name);
PUBLIC void assertion_failure(char *exp, char *file, char *base_file, int line);

#endif
