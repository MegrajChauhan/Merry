#include "merry_time.h"
#include <stdlib.h>

// This module is specifically for internal components to use
mret_t merry_get_time(MerryTime *time)
{
#ifdef _USE_LINUX_
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time->seconds = tv.tv_sec;
    time->microseconds = tv.tv_usec;
    return RET_SUCCESS;
#endif

#ifdef _USE_WIN_
    SYSTEMTIME st;
    GetSystemTime(&st);
    time->seconds = st.wSecond;
    time->microseconds = st.wMilliseconds * 1000;
    return RET_SUCCESS;
#endif
}

mret_t merry_sleep(msize_t seconds)
{
#ifdef _USE_LINUX_
    sleep(seconds);
    return RET_SUCCESS;
#endif

#ifdef _USE_WIN_
    Sleep(seconds * 1000);
    return RET_SUCCESS;
#endif
}

mret_t merry_sleep_us(msize_t microseconds)
{
#ifdef _USE_LINUX_
    usleep(microseconds);
    return RET_SUCCESS;
#endif

#ifdef _USE_WIN_
    Sleep(microseconds / 1000);
    return RET_SUCCESS;
#endif
}

MerryTime merry_get_time_difference(MerryTime *t1, MerryTime *t2)
{
    MerryTime diff;

    diff.seconds = abs(t2->seconds - t1->seconds);
    diff.microseconds = abs(t2->microseconds - t1->microseconds);

    return diff;
}

MerryTime merry_get_time_difference_from(MerryTime *time)
{
    MerryTime currentTime;
    merry_get_time(&currentTime);
    return merry_get_time_difference(time, &currentTime);
}