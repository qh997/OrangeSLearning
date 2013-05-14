
#include "const.h"
#include "proto.h"
#include "global.h"

/*****************************************************************************/
 //* FUNCTION NAME: clock_handler
 //*     PRIVILEGE: 1
 //*   RETURN TYPE: void
 //*    PARAMETERS: int irq
 //*   DESCRIPTION: 时钟中断处理
/*****************************************************************************/
PUBLIC void clock_handler(int irq)
{
    if (++ticks >= MAX_TICKS)
        ticks = 0;

    if (p_proc_ready->ticks)
        p_proc_ready->ticks--;

    /* 如果有键按下则通知 TTY 进程有中断发生 */
    if (key_pressed)
        inform_int(TASK_TTY);

    /* 中断重入，直接返回 */
    if (k_reenter != 0)
        return;

    if (p_proc_ready->ticks > 0)
        return;

    schedule(); // 进程调度
}

/*****************************************************************************/
 //* FUNCTION NAME: milli_delay
 //*     PRIVILEGE: 1 ~ 3
 //*   RETURN TYPE: void
 //*    PARAMETERS: int milli_sec
 //*   DESCRIPTION: 延时
/*****************************************************************************/
PUBLIC void milli_delay(int milli_sec)
{
    int t = get_ticks();
    while (((get_ticks() - t) * 1000 / HZ) < milli_sec);
}

/*****************************************************************************/
 //* FUNCTION NAME: init_clock
 //*     PRIVILEGE: 1
 //*   RETURN TYPE: void
 //*    PARAMETERS: void
 //*   DESCRIPTION: 初始化
/*****************************************************************************/
PUBLIC void init_clock()
{
    /* 初始化 8253 PIT */
    out_byte(TIMER_MODE, RATE_GENERATOR);
    out_byte(TIMER0, (u8)(TIMER_FREQ / HZ));
    out_byte(TIMER0, (u8)((TIMER_FREQ / HZ) >> 8));

    /* 设定时钟中断处理程序 */
    put_irq_handler(CLOCK_IRQ, clock_handler);
    enable_irq(CLOCK_IRQ);
}
