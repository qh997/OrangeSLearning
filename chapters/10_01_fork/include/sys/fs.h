#ifndef _KERL__FS_H_
#define _KERL__FS_H_

struct dev_drv_map {
    int driver_nr;
};

#define MAGIC_V1 0x111

struct super_block {
    u32 magic;             // 魔数
    u32 nr_inodes;         // inode 数
    u32 nr_sects;          // 扇区（sector）数
    u32 nr_imap_sects;     // inode map 占用扇区数
    u32 nr_smap_sects;     // sector map 占用扇区数
    u32 nr_inode_sects;    // inode array 占用扇区数
    u32 n_1st_sect;        // 数据区开始扇区号
    u32 root_inode;        // 根目录 inode 号
    u32 inode_size;        // INODE_SIZE
    u32 inode_isize_off;   // offset of 'struct inode::i_size'
    u32 inode_start_off;   // offset of 'struct inode::i_start_sect'
    u32 dir_ent_size;      // DIR_ENTRY_SIZE
    u32 dir_ent_inode_off; // offset of 'struct dir_entyr::inode_nr'
    u32 dir_ent_fname_off; // offset of 'struct dir_entyr::name'

    /* only present in memory */
    int sb_dev;
};
#define SUPER_BLOCK_SIZE 56

struct inode {
    u32 i_mode;       // 文件类型
    u32 i_size;       // 文件大小（字节）
    u32 i_start_sect; // 文件开始扇区
    u32 i_nr_sects;   // 文件占用扇区
    u8  _unused[16];  // stuff for alignment

    /* only present in memory */
    int i_dev;
    int i_cnt;
    int i_num;
};
#define INODE_SIZE 32

#define MAX_FILENAME_LEN 12
struct dir_entry {
    int inode_nr;
    char name[MAX_FILENAME_LEN];
};
#define DIR_ENTRY_SIZE sizeof(struct dir_entry)

struct file_desc {
    int fd_mode;
    int fd_pos;
    int fd_cnt;
    struct inode *fd_inode;
};

#define RD_SECT(dev, sect_nr) \
    rw_sector(DEV_READ, dev, (sect_nr) * SECTOR_SIZE, SECTOR_SIZE, TASK_FS, fsbuf)
#define WR_SECT(dev, sect_nr) \
    rw_sector(DEV_WRITE, dev, (sect_nr) * SECTOR_SIZE, SECTOR_SIZE, TASK_FS, fsbuf)

/*   FS 结构
 *   ┏━━━━━━━━━━━━━━━━━┓
 *   ┃       ...       ┃
 *   ┃       ...       ┃ 
 *   ┃       ...       ┃
 *   ┃      datas      ┃
 *   ┣━━━━━━━━━━━━━━━━━┫
 *   ┃       ...       ┃
 *   ┃     root dir    ┃ 2048 sectors
 *   ┣━━━━━━━━━━━━━━━━━┫
 *   ┃       ...       ┃ 
 *   ┃   inode array   ┃ m sectors
 *   ┣━━━━━━━━━━━━━━━━━┫
 *   ┃       ...       ┃ 
 *   ┃    sector map   ┃ n sectors
 *   ┣━━━━━━━━━━━━━━━━━┫
 *   ┃    inode map    ┃ 1 sector
 *   ┣━━━━━━━━━━━━━━━━━┫
 *   ┃   super block   ┃ 1 sector
 *   ┣━━━━━━━━━━━━━━━━━┫
 *   ┃   boot sector   ┃ 1 sector
 *   ┗━━━━━━━━━━━━━━━━━┛
 */

#endif
