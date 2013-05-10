#include "type.h"
#include "sys/proto.h"
#include "sys/const.h"
#include "sys/protect.h"
#include "sys/global.h"

PRIVATE void spurious_irq(int irq);

PUBLIC void init_8259A()
{
    out_byte(INT_M_CTL, 0x11);                // 主8259, ICW1
    out_byte(INT_S_CTL, 0x11);                // 从8259, ICW1
    out_byte(INT_M_CTLMASK, INT_VECTOR_IRQ0); // 主8259, ICW2
    out_byte(INT_S_CTLMASK, INT_VECTOR_IRQ8); // 从8259, ICW2
    out_byte(INT_M_CTLMASK, 0x04);            // 主8259, ICW3
    out_byte(INT_S_CTLMASK, 0x02);            // 从8259, ICW3
    out_byte(INT_M_CTLMASK, 0x01);            // 主8259, ICW4
    out_byte(INT_S_CTLMASK, 0x01);            // 从8259, ICW4
    out_byte(INT_M_CTLMASK, 0xFF);            // 主8259, OCW1
    out_byte(INT_S_CTLMASK, 0xFF);            // 从8259, OCW1

    /* 初始化中断向量表 */
    for (int i = 0; i < NR_IRQ; i++)
        irq_table[i] = spurious_irq;
}

PUBLIC void put_irq_handler(int irq, irq_handler handler)
{
    disable_irq(irq);
    irq_table[irq] = handler;
}

PRIVATE void spurious_irq(int irq)
{
    disp_str("spurious_irq: ");
    disp_int(irq);
    disp_str("\n");
}
