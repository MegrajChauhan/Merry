/*
 * MIT License
 *
 * Copyright (c) 2024 MegrajChauhan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _MERRY_TIME_
#define _MERRY_TIME_

#include <sys/time.h>
#include <time.h>
#include <unistd.h>
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