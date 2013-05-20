#include "const.h"
#include "global.h"
#include "proto.h"
#include "string.h"

#include "elf.h"

PUBLIC int do_exec()
{
    int name_len = mm_msg.NAME_LEN;
    int src = mm_msg.source;
    assert(name_len < MAX_PATH);

    char pathname[MAX_PATH];
    phys_copy(
        (void *)va2la(TASK_MM, pathname),
        (void *)va2la(src, mm_msg.PATHNAME),
        name_len
    );
    pathname[name_len] = 0;

    struct stat s;
    int ret = stat(pathname, &s);
    if (0 != ret) {
        printl("{MM} MM::do_exec()::stat() returns error. %s", pathname);
        return -1;
    }

    /* 将被执行文件读入 MM 缓冲区 */
    int fd = open(pathname, O_RDWR);
    if (-1 == fd)
        return -1;
    assert(s.st_size < MMBUF_SIZE);
    read(fd, mmbuf, s.st_size);
    close(fd);

    /* 复制程序到内存 */
    Elf32_Ehdr *elf_hdr = (Elf32_Ehdr *)(mmbuf);
    for (int i = 0; i < elf_hdr->e_phnum; i++) {
        Elf32_Phdr *prog_hdr =
            (Elf32_Phdr *)(
                mmbuf +                    // 0
                elf_hdr->e_phoff +         // Program header table 在文件中的偏移量
                (i * elf_hdr->e_phentsize) // i 个 Program header 的大小
            );
        if (PT_LOAD == prog_hdr->p_type) { // 可载入的段
            assert(prog_hdr->p_vaddr + prog_hdr->p_memsz < PROC_IMAGE_SIZE_DEFAULT);
            phys_copy(
                (void *)va2la(src, (void *)prog_hdr->p_vaddr),      // 该段第一个字节在内存中的虚拟地址
                (void *)va2la(TASK_MM, mmbuf + prog_hdr->p_offset), // 该段第一个字节在文件中的偏移
                prog_hdr->p_filesz                                  // 段在文件中的长度
            );
        }
    }

    /* 复制参数栈 */
    int orig_stack_len = mm_msg.BUF_LEN;
    char stackcopy[PROC_ORIGIN_STACK];
    phys_copy(
        (void *)va2la(TASK_MM, stackcopy),
        (void *)va2la(src, mm_msg.BUF),
        orig_stack_len
    );

    /* 从堆栈区顶部开始 */
    u8 *orig_stack = (u8 *)(PROC_IMAGE_SIZE_DEFAULT - PROC_ORIGIN_STACK);

    /* 重置指针 */
    int delta = (int)orig_stack - (int)mm_msg.BUF; // 指针变化量
    int argc = 0;
    if (orig_stack_len) {
        char **q = (char **)stackcopy;
        for (; *q != 0; q++, argc++)
            *q += delta;
    }

    /* 复制参数栈到内存 */
    phys_copy(
        (void *)va2la(src, orig_stack),    // 进程堆栈区顶部
        (void *)va2la(TASK_MM, stackcopy), // MM 中准备好的参数栈
        orig_stack_len
    );

    /* main 函数参数 */
    proc_table[src].regs.ecx = argc;
    proc_table[src].regs.eax = (u32)orig_stack;

    /* eip/esp */
    proc_table[src].regs.eip = elf_hdr->e_entry;
    proc_table[src].regs.esp = PROC_IMAGE_SIZE_DEFAULT - PROC_ORIGIN_STACK;

    strcpy(proc_table[src].name, pathname);

    return 0;
}
