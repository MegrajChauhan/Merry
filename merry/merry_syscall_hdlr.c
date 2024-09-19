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
    merry_update_errno();
    c->registers[Mb] = merry_get_errno();
    free(_name);
}

void merry_exec_syscall(MerryCore *c)
{
    switch (c->registers[Ma])
    {
#ifdef _USE_LINUX_
    case SYS_open:
        merry_syscall_open(c);
        break;
#endif
    default:
#ifdef _USE_LINUX_
        c->registers[Ma] = syscall(c->registers[Ma], c->registers[Mb], c->registers[M2], c->registers[M3], c->registers[M4], c->registers[M5]);
#endif
        merry_update_errno();
        c->registers[Mb] = merry_get_errno();
    }
}