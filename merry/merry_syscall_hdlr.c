#include "merry_syscall_hdlr.h"

merry_syscall(open)
{
    // We need to get the filename or the path and we don't care about anything else
    mstr_t _name = merry_dmemory_get_bytes_maybe_over_multiple_pages_upto(c->data_mem, c->registers[M1], 0);
    if (_name == NULL)
    {
        // malloc might have failed but...
        merry_set_errno(MERRY_SYSCALLERR);
        c->registers[Mb] = MERRY_SYSCALLERR;
        return;
    }
#ifdef _USE_LINUX_
    c->registers[Ma] = syscall(c->registers[Ma], _name, c->registers[M2], c->registers[M3], c->registers[M4], c->registers[M5]);
#endif
    // merry_update_errno();
    c->registers[Mb] = merry_get_errno();
    free(_name);
}

merry_syscall(read)
{
    register msize_t len = c->registers[M3];
    mbyte_t arr[len];
#ifdef _USE_LINUX_
    c->registers[Ma] = syscall(c->registers[Ma],c->registers[M1], arr, len);
#endif
    // merry_update_errno();
    c->registers[Mb] = merry_get_errno();
    if (merry_dmemory_write_bytes_maybe_over_multiple_pages(c->data_mem, c->registers[M2], len, arr) == RET_FAILURE)
    {
        merry_set_errno(MERRY_SYSCALLERR);
        c->registers[Mb] = MERRY_SYSCALLERR;
        c->registers[Ma] = -1;
    }
}

merry_syscall(write)
{
    register msize_t len = c->registers[M3];
    mbptr_t arr = merry_dmemory_get_bytes_maybe_over_multiple_pages(c->data_mem, c->registers[M2], len);
    if (arr == NULL)
    {
        // malloc might have failed but...
        merry_set_errno(MERRY_SYSCALLERR);
        c->registers[Mb] = MERRY_SYSCALLERR;
        return;
    }
#ifdef _USE_LINUX_
    c->registers[Ma] = syscall(c->registers[Ma], c->registers[M1], arr, len);
#endif
    // merry_update_errno();
    c->registers[Mb] = merry_get_errno();
}

void merry_exec_syscall(MerryCore *c)
{
    switch (c->registers[Ma])
    {
#ifdef _USE_LINUX_
    case SYS_open:
        merry_syscall_open(c);
        break;
    case SYS_read:
        merry_syscall_read(c);
        break;
    case SYS_write:
        merry_syscall_write(c);
        break;
#endif
    default:
#ifdef _USE_LINUX_
        c->registers[Ma] = syscall(c->registers[Ma], c->registers[Mb], c->registers[M2], c->registers[M3], c->registers[M4], c->registers[M5]);
#endif
        // merry_update_errno();
        c->registers[Mb] = merry_get_errno();
    }
}