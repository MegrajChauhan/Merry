/*
 * Request handler implementation of the Merry VM
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
#ifndef _MERRY_REQUEST_HDLR_
#define _MERRY_REQUEST_HDLR_

#include <merry_request_queue.h>
#include <merry_errors.h>
#include <merry_thread.h>
#include <stdlib.h>

typedef struct MerryRequestHdlr MerryRequestHdlr;

struct MerryRequestHdlr
{
    MerryRequestQueue *queue; // the request queue
    MerryMutex *lock;         // only one thread can pop and push
    MerryCond *host_cond;     // the OS's condition variable
    mbool_t handle_more;      // a flag to see if the handler should accept more request
};

// the request handler doesn't belong to even the OS just like Reader
static MerryRequestHdlr req_hdlr;

mret_t merry_requestHdlr_init(msize_t queue_len, MerryCond *cond);

mret_t merry_requestHdlr_push_request(msize_t req_id, msize_t id, MerryCond *req_cond);

// exclusive for the OS
mbool_t merry_requestHdlr_pop_request(MerryOSRequest *request);

// exclusive for cores to inform the OS of errors quickly
void merry_requestHdlr_panic(merrot_t error);

// Make all cores that were waiting to continue with error
void merry_requestHdlr_kill_requests();

void merry_requestHdlr_destroy();

#endif