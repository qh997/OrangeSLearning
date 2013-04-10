#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "type.h"
#include "protect.h"
#include "proc.h"

#ifdef GLOBAL_VARIABLES_HERE
#undef  EXTERN
#define EXTERN
#endif

EXTERN int        disp_pos;
EXTERN DESCRIPTOR gdt[GDT_SIZE];
EXTERN u8         gdt_ptr[6];
EXTERN GATE       idt[IDT_SIZE];
EXTERN u8         idt_ptr[6];

EXTERN TSS        tss;
EXTERN PROCESS    *p_proc_ready;

extern PROCESS proc_table[];
extern char task_stack[];

#endif
