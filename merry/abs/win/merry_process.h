#ifndef _MERRY_PROCESS_
#define _MERRY_PROCESS_

#include <merry_utils.h>
#include <merry_types.h>
#include <stdlib.h>
#include <windows.h>
#include "merry_temp.h"

// we also need the pid
#define _MERRY_PROCESS_CREATION_SUCCESS_ 0
#define _MERRY_PROCESS_CREATION_FAILURE_ -1

typedef __pid_t mpid_t;
typedef struct MerryProcess MerryProcess;

// This is based on my research and hence maybe wrong or not work at all
struct MerryProcess
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
};

MerryProcess merry_create_process();

// mpid_t merry_get_pid();

#endif