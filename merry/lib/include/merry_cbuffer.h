#ifndef _MERRY_CBUFFER_
#define _MERRY_CBUFFER_

#include "merry_squeue.h"
#include <stdio.h>

typedef struct MerryCBuffer MerryCBuffer;
_MERRY_CREATE_SQUEUE_(char, MerryCQueue);
// The buffer is of char type

struct MerryCBuffer
{
    MerryCQueue *queue; // the buffer
    mcstr_t _dump_to_;  // the buffer is dumped into _dump_to_
};

#endif