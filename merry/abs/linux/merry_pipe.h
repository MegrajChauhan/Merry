#ifndef _MERRY_PIPE_
#define _MERRY_PIPE_

#include "merry_config.h"
#include "merry_types.h"
#include <unistd.h>
#include <stdlib.h>

typedef struct MerryPipe MerryPipe;

#define _MERRY_CLOREND_ 0 
#define _MERRY_CLOWEND_ 1

struct MerryPipe
{
    union
    {
        int pfd[2]; // the file descriptors
        struct
        {
            int _read_fd;
            int _write_fd;
        };
    };
    mbool_t _in_use;
    mbool_t _rclosed;
    mbool_t _wclosed;
};

MerryPipe *merry_open_pipe();

void merry_pipe_close_one_end(MerryPipe *pipe, int end);

void merry_pipe_close_both_ends(MerryPipe *pipe);

mret_t merry_pipe_reopen(MerryPipe *_pipe);

void merry_destroy_pipe(MerryPipe *pipe);

#endif