#ifndef _MERRY_PIPE_
#define _MERRY_PIPE_

#include <windows.h>
#include <stdlib.h>
#include "merry_errno.h"
#include "merry_types.h"
#include "merry_config.h"

typedef struct MerryPipe MerryPipe;

#define _MERRY_CLOREND_ 0
#define _MERRY_CLOWEND_ 1

#define _MERRY_IN_RFD_ 0 // prevent read fd inheritance
#define _MERRY_IN_WRF_ 1 // prevent write fd inheritance

struct MerryPipe
{
    union
    {
        HANDLE pfd[2]; // The pipe handles (in Windows, HANDLE is used instead of int)
        struct
        {
            HANDLE _read_fd;
            HANDLE _write_fd;
        };
    };
    bool _in_use; // Using standard bool, you can define a custom mbool_t if needed.
};

MerryPipe *merry_open_pipe();

void merry_pipe_close_one_end(MerryPipe *pipe, int end);

void merry_destroy_pipe(MerryPipe *pipe);

void merry_pipe_close_both_ends(MerryPipe *pipe);

mret_t merry_pipe_reopen(MerryPipe *_pipe);

void merry_pipe_prevent_inheritance(MerryPipe *pipe, int flag);

#endif