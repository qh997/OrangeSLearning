#ifndef _KERL__PROTO_H_
#define _KERL__PROTO_H_

#include "const.h"
#include "type.h"
#include "tty.h"
#include "proc.h"

/* kliba.asm */
PUBLIC void disp_str(char *info);
PUBLIC void disp_color_str(char *info, int color);
PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8 in_byte(u16 port);
PUBLIC int disable_irq(int irq);
PUBLIC void enable_irq(int irq);
PUBLIC void disable_int();
PUBLIC void enable_int();

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

/* main.c */
PUBLIC int kernel_main();
void TestA();
void TestB();
void TestC();

/* clock.c */
PUBLIC void milli_delay(int milli_sec);
PUBLIC void init_clock();

/* proc.c */
PUBLIC void schedule();
PUBLIC int sys_get_ticks();

/* syscall.asm */
PUBLIC int get_ticks();
PUBLIC int write(char *buf, int len);

/* keyboard.c */
PUBLIC void init_keyboard();
PUBLIC void keyboard_read();

/* tty.c */
PUBLIC void task_tty();
PUBLIC void in_process(TTY *p_tty, u32 key);
PUBLIC int sys_write(char *buf, int len, PROCESS *p_proc);

/* console.c */
PUBLIC void init_screen(TTY *p_tty);
PUBLIC int is_current_console(CONSOLE *p_con);
PUBLIC void out_char(CONSOLE *p_con, char ch);
PUBLIC void select_console(int nr_console);
PUBLIC void scroll_screen(CONSOLE *p_con, int direction);

/* vsprintf.c */
PUBLIC int vsprintf(char *buf, const char *fmt, va_list args);

/* printf.c */
PUBLIC int printf(const char *fmt, ...);

#endif
