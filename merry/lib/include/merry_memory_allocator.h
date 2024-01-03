/*
 * Memory allocator for the Merry VM
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
// This is the worst memory allocator anyone can write but for now at least it works.

#ifndef _MERRY_MEMORY_ALLOCATOR_
#define _MERRY_MEMORY_ALLOCATOR_

#include "../../../sys/merry_thread.h"
#include "../../../sys/merry_mem.h"
#include <unistd.h>

#define _MERRY_ALLOC_MAGIC_NUM_ 0xFFFFFFFFFFFFFFF8

_MERRY_ALWAYS_INLINE msize_t merry_align_size(msize_t size)
{
    return (size + 7) & _MERRY_ALLOC_MAGIC_NUM_;
}

// We only need this allocator to fulfill the requests from Merry
// we only allocate aligned memory
// we don't care about anything else such as colored memory or whatever they are called
// Based on the size of the request, we will decide on what to use: sbrk or mmap

typedef struct MerryAllocator MerryAllocator;

#endif