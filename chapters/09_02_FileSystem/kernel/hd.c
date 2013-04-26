#include "const.h"
#include "proto.h"
#include "hd.h"

PRIVATE void init_hd();
PRIVATE void hd_identify(int drive);
PRIVATE void hd_cmd_out(struct hd_cmd *cmd);
PRIVATE int waitfor(int mask, int val, int timeout);
PRIVATE void interrupt_wait();
PRIVATE void print_identify_info(u16 *hdinfo);

PRIVATE u8 hd_status;
PRIVATE u8 hdbuf[SECTOR_SIZE * 2];

/*****************************************************************************/
//* FUNCTION NAME: task_hd
//*     PRIVILEGE: 1
//*   RETURN TYPE: void
//*    PARAMETERS: void
//*   DESCRIPTION: 硬盘驱动主循环
/*****************************************************************************/
PUBLIC void task_hd()
{
    MESSAGE msg;

    init_hd();

    while (1) {
        send_recv(RECEIVE, ANY, &msg);

        int src = msg.source;

        switch (msg.type) {
            case DEV_OPEN:
                hd_identify(0);
                break;
            default:
                dump_msg("HD driver::unknown msg", &msg);
                spin("FS::main_loop (invalid msg.type");
                break;
        }

        send_recv(SEND, src, &msg);
    }
}

/*****************************************************************************/
//* FUNCTION NAME: hd_handler
//*     PRIVILEGE: 0
//*   RETURN TYPE: void
//*    PARAMETERS: int irq
//*   DESCRIPTION: 中断处理程序
/*****************************************************************************/
PUBLIC void hd_handler(int irq)
{
    hd_status = in_byte(REG_STATUS);

    inform_int(TASK_HD);
}

/*****************************************************************************/
//* FUNCTION NAME: init_hd
//*     PRIVILEGE: 1
//*   RETURN TYPE: void
//*    PARAMETERS: void
//*   DESCRIPTION: 硬盘驱动初始化
/*****************************************************************************/
PRIVATE void init_hd()
{
    u8 *pNrDrives = (u8 *)(0x475);
    printl("NrDrives:%d.\n", *pNrDrives);
    assert(*pNrDrives);

    put_irq_handler(AT_WINI_IRQ, hd_handler);
    enable_irq(CASCADE_IRQ);
    enable_irq(AT_WINI_IRQ);
}

/*****************************************************************************/
//* FUNCTION NAME: hd_identify
//*     PRIVILEGE: 1
//*   RETURN TYPE: void
//*    PARAMETERS: int drive
//*   DESCRIPTION: 硬盘识别并打印硬盘信息
/*****************************************************************************/
PRIVATE void hd_identify(int drive)
{
    struct hd_cmd cmd;
    cmd.device = MAKE_DEVICE_REG(0, drive, 0);
    cmd.command = ATA_IDENTIFY;
    hd_cmd_out(&cmd);
    interrupt_wait();
    port_read(REG_DATA, hdbuf, SECTOR_SIZE);

    print_identify_info((u16 *)hdbuf);
}

/*****************************************************************************/
//* FUNCTION NAME: hd_cmd_out
//*     PRIVILEGE: 1
//*   RETURN TYPE: void
//*    PARAMETERS: struct hd_cmd *cmd
//*   DESCRIPTION: 硬盘命令输出
/*****************************************************************************/
PRIVATE void hd_cmd_out(struct hd_cmd *cmd)
{
    if (!waitfor(STATUS_BSY, 0, HD_TIMEOUT))
        panic("hd error.");

    out_byte(REG_DEV_CTRL, 0);

    out_byte(REG_FEATURES, cmd->features);
    out_byte(REG_NSECTOR, cmd->count);
    out_byte(REG_LBA_LOW, cmd->lba_low);
    out_byte(REG_LBA_MID, cmd->lba_mid);
    out_byte(REG_LBA_HIGH, cmd->lba_high);
    out_byte(REG_DEVICE, cmd->device);

    out_byte(REG_CMD, cmd->command);
}

/*****************************************************************************/
//* FUNCTION NAME: waitfor
//*     PRIVILEGE: 1
//*   RETURN TYPE: void
//*    PARAMETERS: int mask
//*                int val
//*                int timeout
//*   DESCRIPTION: 等待硬盘处于某一状态
/*****************************************************************************/
PRIVATE int waitfor(int mask, int val, int timeout)
{
    int t = get_ticks();

    while (((get_ticks() - t) * 1000 / HZ) < timeout)
        if ((in_byte(REG_STATUS) & mask) == val)
            return 1;
    return 0;
}

/*****************************************************************************/
//* FUNCTION NAME: interrupt_wait
//*     PRIVILEGE: 1
//*   RETURN TYPE: void
//*    PARAMETERS: void
//*   DESCRIPTION: 等待硬盘中断发生
/*****************************************************************************/
PRIVATE void interrupt_wait()
{
    MESSAGE msg;
    send_recv(RECEIVE, INTERRUPT, &msg);
}

/*****************************************************************************/
//* FUNCTION NAME: print_identify_info
//*     PRIVILEGE: 1
//*   RETURN TYPE: void
//*    PARAMETERS: u16 *hdinfo
//*   DESCRIPTION: 打印硬盘信息
/*****************************************************************************/
PRIVATE void print_identify_info(u16 *hdinfo)
{
    char s[64];

    struct iden_info_ascii {
        int idx;
        int len;
        char *desc;
    } iinfo[] = {
        {10, 20, "HD SN"},
        {27, 40, "HD Model"},
    };

    for (int k = 0; k < sizeof(iinfo)/sizeof(iinfo[0]); k++) {
        char *p = (char *)&hdinfo[iinfo[k].idx];
        int i;
        for (i = 0; i < iinfo[k].len/2; i++) {
            s[i * 2 + 1] = *p++;
            s[i * 2] = *p++;
        }
        s[i * 2] = 0;
        printl("%s: %s\n", iinfo[k].desc, s);
    }

    int capabilities = hdinfo[49];
    printl("LBA supported: %s\n",
           (capabilities & 0x0200) ? "Yes" : "No");

    int cmd_set_supported = hdinfo[83];
    printl("LBA48 supported: %s\n",
           (cmd_set_supported & 0x0400) ? "Yes" : "No");

    int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
    printl("HD size: %dMB\n", sectors * 512 / (1024 * 1024));
}
