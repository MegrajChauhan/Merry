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

typedef struct MerryInstQueue MerryInstQueue;
typedef struct MerryInstQueueNode MerryInstQueueNode;

#include "../../lib/include/merry_queue.h"
#include "merry_inst.h"
#include <stdlib.h>

_MERRY_CREATE_QUEUE_NODE_NOPTR_(MerryInstruction, MerryInstQueueNode)
_MERRY_CREATE_QUEUE_NOPTR_(MerryInstQueue, MerryInstQueueNode)

MerryInstQueue *merry_inst_queue_init(msize_t queue_len);

mret_t merry_inst_queue_push_instruction(MerryInstQueue *queue, MerryInstruction inst);

mret_t merry_inst_queue_pop_instruction(MerryInstQueue *queue, MerryInstruction *inst);

void merry_inst_queue_destroy(MerryInstQueue *queue);

// Further feature addition maybe in the future
#endif