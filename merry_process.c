#include "merry_process.h"

MerryProcess merry_create_process()
{
    MerryProcess p;
    p.pid = fork();
    if (p.pid == -1)
        p.pid = _MERRY_PROCESS_CREATION_FAILURE_;
    return p;
}

mpid_t merry_get_pid()
{
    return getpid();
}