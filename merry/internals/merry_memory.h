/*
 * Memory of the Merry VM
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
#ifndef _MERRY_MEMORY_
#define _MERRY_MEMORY_

/*
 The memory is going to be an array of bytes. It will follow the page model while using the endianness of the host to make things faster.
*/
#include "merry_internals.h"
#include "../../sys/merry_mem.h"
#include "../../sys/merry_thread.h" // memory needs to be thread safe
#include "../lib/include/merry_memory_allocator.h"

#define _MERRY_MEMORY_PGALLOC_MAP_PAGE_ _MERRY_MEM_GET_PAGE_(_MERRY_MEMORY_ADDRESSES_PER_PAGE_, _MERRY_PROT_DEFAULT_, _MERRY_FLAG_DEFAULT_)
#define _MERRY_MEMORY_PGALLOC_UNMAP_PAGE_(address) _MERRY_MEM_GIVE_PAGE_(address, _MERRY_MEMORY_ADDRESSES_PER_PAGE_)

// declarations
typedef struct MerryMemPageDetails MerryMemPageDetails; // the details about a memory page
typedef struct MerryMemPage MerryMemPage;               // the memory page
typedef struct MerryMemory MerryMemory;                 // the memory that manages these pages

struct MerryMemPageDetails
{
    // Atomic operations done to the memory:
    // As i cannot think of other ways to provide atomic accesses to data, all i can do is make the page lockable.
    // The locked page can be accessed by only the locker for the time period of the operation making the entire page inaccessible to other cores which is
    // wanted and a waste of time and resources.
    mbool_t _is_locked; // is the page locked?
    msize_t _locker;    // this is like the key to allow the operator to keep accessing the page atomically
};

struct MerryMemPage
{
    MerryMemPageDetails details; // the page details
    mbptr_t address_space;       // the actual memory of the page
};

struct MerryMemory
{
    MerryMemPage **pages;    // the pages
    msize_t number_of_pages; // the number of pages
};

#endif