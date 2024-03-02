/*
 * Configuration for the Merry VM
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
#ifndef _MERRY_CBUFFER_
#define _MERRY_CBUFFER_

/*
  This module is left incomplete for now.
  We will use simple input method for now and in future introduce buffering if needed.
  This module could also be used as a framework for something in future.
  SQueue will come in handy in many modules in the future
*/

#include "merry_squeue.h"
#include <stdio.h>
#include <string.h>

typedef struct MerryCBuffer MerryCBuffer;
_MERRY_CREATE_SQUEUE_(char, MerryCQueue);
// The buffer is of char type

#define _MERRY_CBUFFER_LEN_ 256 // 256 bytes should be enough as the OS will do the buffering for us

struct MerryCBuffer
{
    MerryCQueue *queue; // the buffer
    mcstr_t _dump_to_;  // the buffer is dumped into _dump_to_
};

MerryCBuffer *merry_init_cbuffer(mcstr_t _dump_to_);

void merry_destroy_cbuffer(MerryCBuffer *buf);

// When writing to the buffer, it doesn't matter if it is full
// If the buffer gets full or becomes full, we simply return
// There is no error at the buffer being full, we just stop storing the data

// either _to_write_ is completely written or we write only until the buffer is full
void merry_cbuffer_write(MerryCBuffer *buffer, mcstr_t _to_write_);

void merry_cbuffer_write_byte(MerryCBuffer *buffer, char _to_write_);

// read just 1 byte
mbool_t merry_cbuffer_read_byte(MerryCBuffer *buffer, mstr_t _store_in);

#endif