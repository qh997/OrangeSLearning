#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "global.h"
#include "proc.h"

/* 进程表 */
PUBLIC PROCESS proc_table[NR_TASKS];

/* 进程栈 */
PUBLIC char task_stack[STACK_SIZE_TOTAL];

/* 任务表 */
PUBLIC TASK task_table[NR_TASKS] = {
    {TestA, STACK_SIZE_TESTA, "TestA"},
    {TestB, STACK_SIZE_TESTB, "TestB"},
    {TestC, STACK_SIZE_TESTC, "TestC"},
};

/* 中断向量 */
PUBLIC irq_handler irq_table[NR_IRQ];

PUBLIC system_call sys_call_table[NR_SYS_CALL] = {
    sys_get_ticks,
};
