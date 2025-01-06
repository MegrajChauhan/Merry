#ifndef _MERRY_PROCESS_
#define _MERRY_PROCESS_

#include <merry_utils.h>
#include <merry_types.h>
#include <stdlib.h>
#include <unistd.h>
#include "merry_errno.h"
#include "merry_console.h"

// we also need the pid
#define _MERRY_PROCESS_CREATION_SUCCESS_ 0
#define _MERRY_PROCESS_CREATION_FAILURE_ -1

typedef __pid_t mpid_t;
typedef struct MerryProcess MerryProcess;

struct MerryProcess
{
    mpid_t pid;
};

mbool_t merry_create_process(MerryProcess *p);

mpid_t merry_get_pid();

#endif