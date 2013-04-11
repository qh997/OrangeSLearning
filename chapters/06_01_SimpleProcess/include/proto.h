#ifndef _PROTO_H_
#define _PROTO_H_

#include "const.h"
#include "type.h"

PUBLIC void disp_str(char *info);
PUBLIC void disp_color_str(char *info, int color);
PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8 in_byte(u16 port);
PUBLIC void init_prot();
PUBLIC void init_8259A();
PUBLIC void disp_int(int input);
PUBLIC void delay(int time);
void restart();
void TestA();
void TestB();
void TestC();

#endif
