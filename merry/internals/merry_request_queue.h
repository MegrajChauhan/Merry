/*
 * Request queue of the Merry VM
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
#ifndef _MERRY_OS_QUEUE_
#define _MERRY_OS_QUEUE_

#include "../lib/include/merry_queue.h"
#include "merry_request.h"

_MERRY_CREATE_QUEUE_NODE_(MerryOSRequest, MerryRequestNode)

_MERRY_CREATE_QUEUE_(MerryRequestQueue, MerryRequestNode)

typedef struct MerryRequestNode MerryRequestNode;
typedef struct MerryRequestQueue MerryRequestQueue;

MerryRequestQueue *merry_request_queue_init(msize_t number_of_requests);

_MERRY_ALWAYS_INLINE mbool_t merry_is_queue_full(MerryRequestQueue *queue)
{
    return _MERRY_IS_QUEUE_FULL_(queue) ? mtrue : mfalse;
}

_MERRY_ALWAYS_INLINE mbool_t merry_is_queue_emtpy(MerryRequestQueue *queue)
{
    return _MERRY_IS_QUEUE_EMPTY_(queue) ? mtrue : mfalse;
}

void merry_request_queue_destroy(MerryRequestQueue *queue);

mbool_t merry_push_request(MerryRequestQueue *queue, MerryCond *_req_cond, msize_t req_num, msize_t id);

mbool_t merry_pop_request(MerryRequestQueue *queue, MerryOSRequest *dest);

void merry_destroy_request_queue(MerryRequestQueue *queue);

void merry_panic_push(MerryRequestQueue *queue, merrot_t error);

#endif