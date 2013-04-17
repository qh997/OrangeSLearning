#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proto.h"
#include "global.h"

PUBLIC void cstart()
{
    disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
             "-----\"cstart\" begins-----\n");

    /* 将 LOADER 中的 GDT 复制到新的 GDT 中 */
    memcpy(
        &gdt,                              // 新的 GDT
        (void *)(*((u32 *)(&gdt_ptr[2]))), // LOADER 中的 GDT 基址
        *((u16 *)(&gdt_ptr[0])) + 1        // LOADER 中的 GDT 界限
    );

    /* 更新 gdtptr */
    u16 *p_gdt_limit = (u16 *)(&gdt_ptr[0]);
    u32 *p_gdt_base = (u32 *)(&gdt_ptr[2]);
    *p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1; // 更新 GDT 界限
    *p_gdt_base = (u32)&gdt;                          // 更新 GDT 基址

    /* 初始化 idtptr */
    u16 *p_idt_limit = (u16 *)(&idt_ptr[0]);
    u32 *p_idt_base = (u32 *)(&idt_ptr[2]);
    *p_gdt_limit = IDT_SIZE * sizeof(GATE) - 1;
    *p_idt_base = (u32)&idt;

    init_prot();

    disp_str("-----\"cstart\" ends-----\n");
}
