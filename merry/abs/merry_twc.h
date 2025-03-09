#ifndef _MERRY_TWC_
#define _MERRY_TWC_

// Two-way channel
#include <merry_config.h>
#include <merry_platform.h>
#include <merry_types.h>
#include <merry_utils.h>
#include <merry_owc.h>
#include <stdlib.h>

typedef struct MerryTWC MerryTWC;

struct MerryTWC
{
    MerryOWC* rc;  
    MerryOWC* wr;


};

#endif