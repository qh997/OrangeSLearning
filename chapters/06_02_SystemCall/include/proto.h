#ifndef _KERL__PROTO_H_
#define _KERL__PROTO_H_

#include "const.h"
#include "type.h"

/* kliba.asm */
PUBLIC void disp_str(char *info);
PUBLIC void disp_color_str(char *info, int color);
PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8 in_byte(u16 port);
PUBLIC int disable_irq(int irq);
PUBLIC void enable_irq(int irq);

/* klib.c */
PUBLIC char *itoa(char *str, int num);
PUBLIC void disp_int(int input);
PUBLIC void delay(int time);

/* protect.c */
PUBLIC void init_prot();
PUBLIC void exception_handler(int vec_no, int err_code, int eip, int cs, int eflags);

/* i8259.c*/
PUBLIC void init_8259A();
PUBLIC void spurious_irq(int irq);
PUBLIC void put_irq_handler(int irq, irq_handler handler);

/* kernel.asm */
void restart();

/* main.c */
PUBLIC int kernel_main();
void TestA();
void TestB();
void TestC();

/* clock.c */
PUBLIC void clock_handler(int irq);

/* proc.c */
PUBLIC int sys_get_ticks();

/* syscall.asm */
PUBLIC int get_ticks();

#endif
