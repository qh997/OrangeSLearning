#ifndef _PROTO_H_
#define _PROTO_H_

#include "const.h"
#include "type.h"

/* klib.asm */
PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8 in_byte(u16 port);
PUBLIC void disp_str(char *info);
PUBLIC void disp_color_str(char *info, int color);
PUBLIC int disable_irq(int irq);
PUBLIC void enable_irq(int irq);

/* protect.c */
PUBLIC void init_prot();

/* klib.c */
PUBLIC char *itoa(char *str, int num);
PUBLIC void disp_int(int input);
PUBLIC void delay(int time);

/* kernel.asm */
void restart();

/* main.c */
void TestA();
void TestB();
void TestC();

/* i8259.c*/
PUBLIC void init_8259A();
PUBLIC void spurious_irq(int irq);
PUBLIC void put_irq_handler(int irq, irq_handler handler);

/* clock.c */
PUBLIC void clock_handler(int irq);

#endif
