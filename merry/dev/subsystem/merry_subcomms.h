#ifndef _MERRY_SUBCOMMS_
#define _MERRY_SUBCOMMS_

#include "merry_pipe.h"
#include <stdlib.h>

typedef struct MerrySubChannel MerrySubChannel;

struct MerrySubChannel
{
    MerryPipe *send_pipe;    // VM writes, process reads
    MerryPipe *receive_pipe; // process writes. VM reads
    mbool_t comms_active;    // is this channel still open
};

MerrySubChannel *merry_create_channel();

mret_t merry_create_channel_given(MerrySubChannel *c);

void merry_inactivate_channel(MerrySubChannel *channel);

mret_t merry_reactivate_channel(MerrySubChannel *channel);

void merry_close_channel(MerrySubChannel *channel);

mret_t merry_channel_read(MerrySubChannel *channel, mstr_t* buf, msize_t buf_len);

mret_t merry_channel_write(MerrySubChannel *channel, mstr_t buf, msize_t buf_len);

void merry_config_channel(MerrySubChannel *channel);

#endif