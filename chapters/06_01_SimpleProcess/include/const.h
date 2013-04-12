#ifndef _KERL__CONST_H_
#define _KERL__CONST_H_

#include "protect.h"

#define EXTERN extern
#define PUBLIC
#define PRIVATE static

/* GDT 和 IDT 中描述符数目 */
#define GDT_SIZE 128
#define IDT_SIZE 256

/* 权限 */
#define PRIVILEGE_KRNL 0
#define PRIVILEGE_TASK 1
#define PRIVILEGE_USER 3

/* RPL */
#define RPL_KRNL SA_RPL0
#define RPL_TASK SA_RPL1
#define RPL_USER SA_RPL3

/* 8259A ports */
#define INT_M_CTL 0x20
#define INT_S_CTL 0xA0
#define INT_M_CTLMASK 0x21
#define INT_S_CTLMASK 0xA1

/* Hardware interrupts */
#define NR_IRQ 16
#define CLOCK_IRQ 0

#endif