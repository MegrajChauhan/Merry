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
 BIG PROBLEM:
 Since making the atomic operations at the memory level is going to be very tough and complicated, we have to implement it at the CPU level.
 The only way that is coming to mind right now is by blocking every other core during atomic memory accesses. If i can come up with a way to make things work
 without hampering the performance, i will implement it right away but at memory level, it is very very complicated.
*/

/*
 The memory is going to be an array of bytes. It will follow the page model while using the endianness of the host to make things faster.
*/

#include "merry_internals.h"

#if defined(_WIN64)
#include "..\..\sys\merry_mem.h"
#include "..\..\sys\merry_thread.h" // memory needs to be thread safe
#include "..\includes\merry_errors.h"
#include "..\..\utils\merry_logger.h"
#else
#include "../../sys/merry_mem.h"
#include "../../sys/merry_thread.h" // memory needs to be thread safe
#include "../includes/merry_errors.h"
#endif

#include <stdlib.h>
#include <stdatomic.h>

// declarations
// typedef struct MerryMemPageDetails MerryMemPageDetails; // the details about a memory page
typedef struct MerryMemPage MerryMemPage; // the memory page
typedef struct MerryMemory MerryMemory;   // the memory that manages these pages
typedef struct MerryAddress MerryAddress; // an internal struct

#define _MERRY_MEMORY_PGALLOC_MAP_PAGE_ _MERRY_MEM_GET_PAGE_(_MERRY_MEMORY_ADDRESSES_PER_PAGE_, _MERRY_PROT_DEFAULT_, _MERRY_FLAG_DEFAULT_)
#define _MERRY_MEMORY_PGALLOC_UNMAP_PAGE_(address) _MERRY_MEM_GIVE_PAGE_(address, _MERRY_MEMORY_ADDRESSES_PER_PAGE_)

#define _MERRY_MEMORY_DEDUCE_ADDRESS_(addr)                                                       \
    {                                                                                             \
        .page = addr / _MERRY_MEMORY_QS_PER_PAGE_, .offset = address % _MERRY_MEMORY_QS_PER_PAGE_ \
    }

struct MerryMemPage
{
    mqptr_t address_space; // the actual memory of the page
    mbool_t _is_locked;
};

struct MerryMemory
{
    // we make pages as pointer to a pointer because copying a list of pointers during reallocation is faster than copying all of the data about every page
    MerryMemPage **pages;    // the pages
    msize_t number_of_pages; // the number of pages
    merrot_t error;          // any error that the Memory encounters
};

struct MerryAddress
{
    unsigned int page;
    unsigned int offset;
};

MerryMemory *merry_memory_init(msize_t num_of_pages);

// instead of allocating new pages, we use already mapped pages
// After mapping, the reader can return the mapped pages while continuing to read in the background
// this was the memory can serve the cores while it is getting populated
// this has its own challanges such as what if there is error in the input file at the end?
// The memory would be serving the data and instructions that were valid but it turns out that the input was wrong to begin with
// Or what if the initial part of the input file needs the last part to even start working?
MerryMemory *merry_memory_init_provided(mqptr_t *mapped_pages, msize_t num_of_pages);

void merry_memory_free(MerryMemory *memory);

/*
 We will provide an extra level of abstraction here.
 The address generally is: PAGE PAGE_OFFSET
 But that is a very painful way of keeping track of address for the CPU.
 Thus the CPU can assume that the addresses are linear and start from 0 while leaving the hard part of computing the actual address to the memory.
*/
mret_t merry_memory_read(MerryMemory *memory, maddress_t address, mqptr_t _store_in);

mret_t merry_memory_write(MerryMemory *memory, maddress_t address, mqword_t _to_write);

mptr_t merry_memory_get_address(MerryMemory *memory, maddress_t address);

#endif