#include "merry_process.h"

mbool_t merry_create_process(MerryProcess *p)
{
    p->pid = fork();
    if (p->pid == -1)
        return mfalse;
    return mtrue;
}

mpid_t merry_get_pid()
{
    return getpid();
}