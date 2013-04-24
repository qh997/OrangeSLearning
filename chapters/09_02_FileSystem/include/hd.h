#ifndef _KERL__HD_H_
#define _KERL__HD_H_
/* Command Block Registers */
/*      MACRO          PORT            DESCRIPTION                     I/O */
#define REG_DATA       0x1F0        /* Data                            I/O */
#define REG_FEATURES   0x1F1        /* Features                          O */
#define REG_ERROR      REG_FEATURES /* Error                           I   */
#define REG_NSECTOR    0x1F2        /* Sector Count                    I/O */
#define REG_LBA_LOW    0x1F3        /* Sector Number / LBA Bits 0-7    I/O */
#define REG_LBA_MID    0x1F4        /* Cylinder Low / LBA Bits 8-15    I/O */
#define REG_LBA_HIGH   0x1F5        /* Cylinder High / LBA Bits 16-23  I/O */
#define REG_DEVICE     0x1F6        /* Drive | Head | LBA bits 24-27   I/O */
#define REG_CMD        0x1F7        /* Command                           O */
#define REG_STATUS     REG_CMD      /* Status                          I   */
#define REG_DEV_CTRL   0x3F6        /* Device Control                    O */
#define REG_ALT_STATUS REG_DEV_CTRL /* Alternate Status                I   */
#define REG_DRV_ADDR   0x3F7        /* Drive Address                   I   */

#define STATUS_BSY  0x80
#define STATUS_DRDY 0x40
#define STATUS_DFSE 0x20
#define STATUS_DSC  0x10
#define STATUS_DRQ  0x08
#define STATUS_CORR 0x04
#define STATUS_IDX  0x02
#define STATUS_ERR  0x01

#define HD_TIMEOUT 10000
#define PARTITION_TABLE_OFFSET 0x1BE
#define ATA_IDENTIFY 0xEC
#define ATA_READ 0x20
#define ATA_WRITE 0x30

#define MAKE_DEVICE_REG(lba, drv, lba_highest) \
    (((lba) << 6) | ((drv) << 4) | ((lba_highest) & 0xF) | 0xA0)

struct hd_cmd {
    u8 features;
    u8 count;
    u8 lba_low;
    u8 lba_mid;
    u8 lba_high;
    u8 device;
    u8 command;
};

#endif
