#ifndef _MERRY_TIME_
#define _MERRY_TIME_

#include <windows.h>
#include <merry_types.h>
#include <merry_utils.h>

typedef struct MerryTime MerryTime;

struct MerryTime
{
    int seconds;
    int microseconds;
};

#define _MERRY_SYS_TICKS_PER_SECONDS_ CLOCKS_PER_SEC

mret_t merry_get_time(MerryTime *time);

mret_t merry_sleep(msize_t seconds);

mret_t merry_sleep_us(msize_t microseconds);

// Get a time difference between t1 and t2
MerryTime merry_get_time_difference(MerryTime *t1, MerryTime *t2);

// Get a time difference from current time with the given time
MerryTime merry_get_time_difference_from(MerryTime *time);

#endif