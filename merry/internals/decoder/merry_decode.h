/*
 * Instruction decoder of the Merry VM
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
#ifndef _MERRY_DECODE_
#define _MERRY_DECODE_

typedef struct MerryDecoder MerryDecoder;

#include "../../../utils/merry_logger.h"
#include "merry_opcodes.h"
#include "inst/merry_inst_queue.h"
#include "inst/merry_exec.h"
#include "../merry_os.h"
#include <stdlib.h>

#define _MERRY_INST_BUFFER_LEN_ 10

struct MerryDecoder
{
    MerryCore *core;        // the host core
    MerryMutex *lock;       // the decoder's lock
    MerryMutex *queue_lock; // the queue's lock
    MerryCond *cond;        // the decoder's condition variable
    mbool_t should_stop;    // stop decoding
    mbool_t provide;        // provide more instruction or not?
    MerryInstQueue *queue;  // the instruction queue
};

MerryDecoder *merry_init_decoder(MerryCore *host);

void merry_decoder_get_inst(MerryDecoder *decoder);

void merry_destroy_decoder(MerryDecoder *decoder);

// Run the decoder
mptr_t merry_decode(mptr_t d);

#endif