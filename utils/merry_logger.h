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
#ifndef _MERRY_LOGGER_
#define _MERRY_LOGGER_

#include <stdio.h>
#include <stdarg.h>
#include "../sys/merry_thread.h"

// #define _MERRY_LOGGER_ENABLED_

struct MerryLogger
{
    FILE *f;
    mbool_t enabled; // enable debugger? creates a file named "merry.log"
    MerryMutex *lock;
};

static struct MerryLogger logger;

mret_t merry_logger_init(mbool_t enable_flag);

void merry_logger_close();

// The logging format is: [Device][Additional Information]: Details
void merry_log(mstr_t _device_, mstr_t _info_, mstr_t _details_);

void merry_llog(mstr_t _device_, mstr_t _info_, mstr_t _msg_, ...);

// definition for devices
#define _OS_ "Manager"
#define _CORE_ "Core"
#define _MEM_ "Memory"
#define _READER_ "Reader"
#define _REQHDLR_ "Request Handler"
#define _DECODER_ "Decoder"

// #define _INFO_INIT_ "Intialization"
// #define _INFO_EXEC_ "Executing"

#endif