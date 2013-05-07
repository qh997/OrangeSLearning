#ifndef _KERL__FS_H_
#define _KERL__FS_H_

struct dev_drv_map {
    int driver_nr;
};

#define MAGIC_V1 0x111

struct super_block {
    u32 magic;             // magic number
    u32 nr_inodes;         // how many inodes
    u32 nr_sects;          // how many sectors
    u32 nr_imap_sects;     // how many inode-map sectors
    u32 nr_smap_sects;     // how many sector-map sectors
    u32 n_1st_sect;        // number of the 1st data sector
    u32 nr_inode_sects;    // how many inode sectors
    u32 root_inode;        // inode nr of root directory
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
    u32 i_mode;       // access mode
    u32 i_size;       // file size
    u32 i_start_sect; // frist sector of the data
    u32 i_nr_sects;   // how many sectors the file occupies
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
    struct inode *fd_onode;
};

#define RD_SECT(dev, sect_nr) \
    rw_sector(DEV_READ, dev, (sect_nr) * SECTOR_SIZE, SECTOR_SIZE, TASK_FS, fsbuf)
#define WR_SECT(dev, sect_nr) \
    rw_sector(DEV_WRITE, dev, (sect_nr) * SECTOR_SIZE, SECTOR_SIZE, TASK_FS, fsbuf)

#endif
