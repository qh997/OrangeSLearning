#include "const.h"
#include "type.h"
#include "proto.h"
#include "global.h"

PRIVATE void init_mm();

PUBLIC void task_mm()
{
    init_mm();

    while (1) {
        send_recv(RECEIVE, ANY, &mm_msg);
        int src = mm_msg.source;
        int reply = 1;

        int msgtype = mm_msg.type;

        switch (msgtype) {
            case FORK:
                mm_msg.RETVAL = do_fork();
                break;

            case EXIT:
                do_exit(mm_msg.STATUS);
                reply = 0;
                break;

            case WAIT:
                do_wait();
                reply = 0;
                break;

            default:
                dump_msg("MM::unknown msg", &mm_msg);
                assert(0);
                break;
        }

        if (reply) {
            mm_msg.type = SYSCALL_RET;
            send_recv(SEND, src, &mm_msg);
        }
    }
}

PRIVATE void init_mm()
{
    struct boot_params bp;
    get_boot_params(&bp);

    memory_size = bp.mem_size;

    printl("{MM} memsize:%dMB\n", memory_size / (1024 * 1024));
}

PUBLIC int alloc_mem(int pid, int memsize)
{
    assert(pid >= (NR_TASKS + NR_NATIVE_PROCS));
    if (memsize > PROC_IMAGE_SIZE_DEFAULT) {
        panic("unsupported memory request: %d. "
              "(should be less than %d)",
              memsize,
              PROC_IMAGE_SIZE_DEFAULT);
    }

    int base = PROCS_BASE +
        (pid - (NR_TASKS + NR_NATIVE_PROCS)) * PROC_IMAGE_SIZE_DEFAULT;

    if (base + memsize >= memory_size)
        panic("memory allocation failed. pid:%d", pid);

    return base;
}
