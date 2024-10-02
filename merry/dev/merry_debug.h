#ifndef _MERRY_DEBUG_
#define _MERRY_DEBUG_

#include "merry_commands.h"
#include "merry_types.h"
#include "merry_config.h"
#include "merry_request_hdlr.h"
#include "merry_thread.h"
#include <stdlib.h>
#ifdef _USE_LINUX_
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#endif
#include <stdatomic.h>
#include <string.h>

typedef struct MerryDebug MerryDebug;

struct MerryDebug
{
#ifdef _USE_LINUX_
    int input_handle, output_handle;
#endif
    MerryCond *cond;
    MerryMutex *lock;
    mbool_t stop;
};

extern void merry_os_dbg_stopped();

MerryDebug *merry_init_debug();

_THRET_T_ merry_start_debugging(mptr_t arg);

void merry_destroy_debug(MerryDebug *dbg);

void merry_send_to_debugger(MerryDebug *dbg, mqword_t op, mqword_t oper);

#endif