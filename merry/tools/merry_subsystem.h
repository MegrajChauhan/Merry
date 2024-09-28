#ifndef _MERRY_SUBSYS_
#define _MERRY_SUBSYS_

#include "merry_subcomms.h"
#include "merry_task_queue.h"
#include "merry_request_hdlr.h"
#include <string.h>

#ifdef _USE_LINUX_
#include <sys/epoll.h>
#endif

typedef struct MerrySubSys MerrySubSys;

#define _MERRY_TASK_QUEUE_DEFAULT_ 20

struct MerrySubSys
{
    MerrySubChannel **channels;
    msize_t subsys_count;
    msize_t subsys_active;
    msize_t subsys_created;
    MerryTaskQueue *queue;
    MerryMutex *lock;
    mbool_t _stop;
    MerryPipe *os_pipe;
};

_MERRY_INTERNAL_ MerrySubSys subsys;

mret_t merry_init_subsys(msize_t _expected_subsys_count);

void merry_subsys_add_ospipe(MerryPipe* p);

msize_t merry_subsys_add_channel();

MerrySubChannel *merry_subsys_get_channel(msize_t id);

mret_t merry_subsys_add_task(msize_t request, MerryCond *cond, mqptr_t _store_in);

void merry_subsys_close_all();

_THRET_T_ merry_subsys_main(mptr_t arg);

mret_t merry_subsys_write(msize_t id, msize_t request, mqword_t arg1, mqword_t arg2, mqword_t arg3, mqword_t arg4);

void merry_destroy_subsys();

void merry_subsys_close_channel(msize_t id);

#endif