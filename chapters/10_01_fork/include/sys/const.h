#ifndef _KERL__CONST_H_
#define _KERL__CONST_H_

#include "config.h"

#define STR_DEFAULT_LEN 1024

#define TRUE 1
#define FALSE 0
#define NULL  0

/* color */
#define BLACK  0x0  /*      0000 */
#define WHITE  0x7  /*      0111 */
#define RED    0x4  /*      0100 */
#define GREEN  0x2  /*      0010 */
#define BLUE   0x1  /*      0001 */
#define FLASH  0x80 /* 1000 0000 */
#define BRIGHT 0x08 /* 0000 1000 */

#define MAKE_COLOR(x,y) ((x<<4) | y) /* MAKE_COLOR(Background,Foreground) */

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

/* Process */
#define SENDING   0x02
#define RECEIVING 0x04
#define WAITING   0x08
#define HANGING   0x10
#define FREE_SLOT 0x20

/* TTY */
#define NR_CONSOLES 3

/* 8259A ports */
#define INT_M_CTL     0x20
#define INT_M_CTLMASK 0x21
#define INT_S_CTL     0xA0
#define INT_S_CTLMASK 0xA1

/* 8253/8254 PIT */
#define TIMER0         0x40     /* I/O port for timer channel 0 */
#define TIMER_MODE     0x43     /* I/O port for timer mode control */
#define RATE_GENERATOR 0x34     /*    00    -    11    -    010   -  0  */
                                /* Counter0 - L then H - rate gen - bin */
#define TIMER_FREQ     1193181L /* clock frequency for timer in PC and AT */
#define HZ             100      /* clock freq (software settable on IBM-PC) */

/* AT keyboard */
#define KB_DATA  0x60
#define KB_CMD   0x64
#define LED_CODE 0xED
#define KB_ACK   0xFA

/* VGA */
#define CRTC_ADDR_REG 0x3D4   /* CRT Controller Registers - Addr Register */
#define CRTC_DATA_REG 0x3D5   /* CRT Controller Registers - Data Register */
#define START_ADDR_H  0xC     /* reg index of video mem start addr (MSB) */
#define START_ADDR_L  0xD     /* reg index of video mem start addr (LSB) */
#define CURSOR_H      0xE     /* reg index of cursor position (MSB) */
#define CURSOR_L      0xF     /* reg index of cursor position (LSB) */
#define V_MEM_BASE    0xB8000 /* base of color video memory */
#define V_MEM_SIZE    0x8000  /* 32K: B8000H -> BFFFFH */

/* Hardware interrupts */
#define NR_IRQ        16
#define CLOCK_IRQ     0
#define KEYBOARD_IRQ  1
#define CASCADE_IRQ   2  /* cascade enable for 2nd AT controller */
#define ETHER_IRQ     3  /* default ethernet interrupt vector */
#define SECONDARY_IRQ 3  /* RS232 interrupt vector for port 2 */
#define RS232_IRQ     4  /* RS232 interrupt vector for port 1 */
#define XT_WINI_IRQ   5  /* xt winchester */
#define FLOPPY_IRQ    6  /* floppy disk */
#define PRINTER_IRQ   7
#define AT_WINI_IRQ   14 /* at winchester */

/* tasks */
#define INVALID_DRIVER -20
#define INTERRUPT      -10
#define TASK_TTY       0
#define TASK_SYS       1
#define TASK_HD        2
#define TASK_FS        3
#define TASK_MM        4
#define INIT           5
#define ANY     (NR_TASKS + NR_PROCS + 10)
#define NO_TASK (NR_TASKS + NR_PROCS + 20)

#define MAX_TICKS 0x7FFFABCD

/* system call */
#define NR_SYS_CALL 2

/* ipc */
#define SEND    1
#define RECEIVE 2
#define BOTH    3

/* magic chars used by `printx' */
#define MAG_CH_PANIC  '\002'
#define MAG_CH_ASSERT '\003'

enum msgtype {
    HARD_INT = 1,

    /* SYS task */
    GET_TICKS,
    GET_PID,
    GET_RTC_TIME,

    /* FS */
    OPEN,
    CLOSE,
    READ,
    WRITE,
    LSEEK,
    STAT,
    UNLINK,

    /* FS & TTY */
    SUSPEND_PROC,
    RESUME_PROC,

    /* MM */
    EXEC,
    WAIT,

    /* FS & MM */
    FORK,
    EXIT,

    /* TTY, SYS, FS, MM, etc */
    SYSCALL_RET,

    /* message type for drivers */
    DEV_OPEN = 1001,
    DEV_CLOSE,
    DEV_READ,
    DEV_WRITE,
    DEV_IOCTL,
};

/* macros for messages */
#define RETVAL   u.m3.m3i1
#define FD       u.m3.m3i1
#define FLAGS    u.m3.m3i1
#define STATUS   u.m3.m3i1
#define REQUEST  u.m3.m3i2
#define CNT      u.m3.m3i2
#define NAME_LEN u.m3.m3i2
#define PID      u.m3.m3i2
#define PROC_NR  u.m3.m3i3
#define BUF_LEN  u.m3.m3i3
#define DEVICE   u.m3.m3i4
#define POSITION u.m3.m3l1
#define PATHNAME u.m3.m3p1
#define BUF      u.m3.m3p2

#define DIOCTL_GET_GEO 1

/* hard drive */
#define SECTOR_SIZE 512
#define SECTOR_BITS (SECTOR_SIZE * 8)
#define SECTOR_SIZE_SHIFT 9

/* major device numbers (corresponding to kernel/global.c::dd_map[]) */
#define DEV_NULL     0
#define DEV_FLOPPY   1
#define DEV_CDROM    2
#define DEV_HD       3
#define DEV_CHAR_TTY 4
#define DEV_SCSI     5

/* make device number from major and minor numbers */
#define MAJOR_SHIFT 8
#define MAKE_DEV(a, b) ((a << MAJOR_SHIFT) | b) /* Device number:
                                                 * 15.......8 7.......0
                                                 * |0000 0000|0000 0000|
                                                 * |  major  |  minor  |
                                                 */

/* separate major/minor number from device munber */
#define MAJOR(x) ((x >> MAJOR_SHIFT) & 0xFF)
#define MINOR(x) (x & 0xFF)

#define MAX_DRIVES 2                                           // 最大硬盘数
#define NR_SUB_PER_PART 16                                     // 扩展分区包含最大逻辑分区数
#define NR_PART_PER_DRIVE 4                                    // 硬盘包含最大扩展分区数（hd1 ~ hd4）
#define NR_PRIM_PER_DRIVE (NR_PART_PER_DRIVE + 1)              // 硬盘包含最大主分区数（hd0 ~ hd4）
#define NR_SUB_PER_DRIVE (NR_SUB_PER_PART * NR_PART_PER_DRIVE) // 硬盘包含最大逻辑分区数
#define MAX_PRIM (MAX_DRIVES * NR_PRIM_PER_DRIVE - 1)          // 最大主分区号
#define MAX_SUBPARTITIONS (NR_SUB_PER_DRIVE * MAX_DRIVES)      // 最大逻辑分区数

/* minor numbers of hard disk */
#define MINOR_hd1a (0x10)
#define MINOR_hd2a (MINOR_hd1a + NR_SUB_PER_PART)

#define ROOT_DEV MAKE_DEV(DEV_HD, MINOR_BOOT)

#define P_PRIMARY  0
#define P_EXTENDED 1

#define QHS_OART 0x99
#define NO_PART  0x00
#define EXT_PART 0x05

#define INVALID_INODE 0
#define ROOT_INODE    1

#define NR_FILES       64
#define NR_FILE_DESC   64
#define NR_SUPER_BLOCK  8
#define NR_INODE       64

/* INODE::i_mode (octal, lower 12 bits reserved) */
#define I_TYPE_MASK     0170000 // 0xF000
#define I_REGULAR       0100000 // 0x8000
#define I_BLOCK_SPECIAL 0060000 // 0x6000 = I_DIRECTORY | I_CHAR_SPECIAL
#define I_DIRECTORY     0040000 // 0x4000
#define I_CHAR_SPECIAL  0020000 // 0x2000
#define I_NAMED_PIPE    0010000 // 0x1000

#define NR_DEFAULT_FILE_SECTS 2048 // 2048 * 512 = 1MB

#define enable_interrupt() __asm__("sti")
#define disable_interrupt() __asm__("cli")

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#define offsetof(TYPE, MEMBER) ((int)&((TYPE *)0)->MEMBER)
#define is_special(m) ((((m) & I_TYPE_MASK) == I_BLOCK_SPECIAL) || \
                      (((m) & I_TYPE_MASK) == I_CHAR_SPECIAL))

#include "proc.h"
#endif
