/*
 * The instruction queue of the Merry VM
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
#ifndef _MERRY_INST_QUEUE_
#define _MERRY_INST_QUEUE_

// #include "../merry_include.h"
// #include "merry_inst.h"
// #include <stdlib.h>

// // define the instruction queue
// typedef struct MerryInstQueue MerryInstQueue;
// typedef struct MerryInstQueueNode MerryInstQueueNode;

// _MERRY_CREATE_QUEUE_NODE_(MerryInstruction, MerryInstQueueNode)
// _MERRY_CREATE_QUEUE_(MerryInstQueue, MerryInstQueueNode)

// MerryInstQueue *merry_inst_queue_init(msize_t queue_len);

// mret_t merry_requestHdlr_push_request(msize_t req_id, msize_t id, MerryCond *req_cond);

// // exclusive for the OS
// mbool_t merry_requestHdlr_pop_request(MerryOSRequest *request);

// // exclusive for cores to inform the OS of errors quickly
// void merry_requestHdlr_panic(merrot_t error);

// // Make all cores that were waiting to continue with error
// void merry_requestHdlr_kill_requests();

// void merry_requestHdlr_destroy();

#endif