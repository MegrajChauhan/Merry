#ifndef _MERRY_SUBSYS_
#define _MERRY_SUBSYS_

#include "merry_subcomms.h"

typedef struct MerrySubSys MerrySubSys;

struct MerrySubSys
{
    MerrySubChannel **channels;
    msize_t subsys_count;
    msize_t subsys_active;
};

_MERRY_INTERNAL_ MerrySubSys subsys;

mret_t merry_init_subsys(msize_t _expected_subsys_count);



void merry_destroy_subsys();

#endif