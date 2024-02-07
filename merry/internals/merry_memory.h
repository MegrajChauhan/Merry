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
#include "../../sys/merry_mem.h"
#include "../../sys/merry_thread.h" // memory needs to be thread safe
// #include "../lib/include/merry_memory_allocator.h" <LEGACY>
#include "../includes/merry_errors.h"
#include "../../utils/merry_logger.h"
#include "imp/merry_extra.h"
#include <stdlib.h>

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

// struct MerryMemPageDetails
// {
//     // Atomic operations done to the memory:
//     // As i cannot think of other ways to provide atomic accesses to data, all i can do is make the page lockable.
//     // The locked page can be accessed by only the locker for the time period of the operation making the entire page inaccessible to other cores which is
//     // unwanted and a waste of time and resources.
//     // I cannot think of other ways to solve this
//     mbool_t _is_locked;   // is the page locked?
//     unsigned int _locker; // this is like the key to allow the operator to keep accessing the page atomically
// };

struct MerryMemPage
{
    mqptr_t address_space; // the actual memory of the page
    // MerryMemPageDetails details; // the page details
    MerryMutex *lock; // Many different pages can be accessed simultaneously
    // MerryCond *cond;
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

// since this module acts as the template for both instruction and data memory, we will need to provide two different types of read/write functions.
// The above read/write functions read/write without locking which is preferred for instruction memory but not preferred for data memory.
mret_t merry_memory_read_lock(MerryMemory *memory, maddress_t address, mqptr_t _store_in);

mret_t merry_memory_write_lock(MerryMemory *memory, maddress_t address, mqword_t _to_write);

mptr_t merry_memory_get_address(MerryMemory *memory, maddress_t address);

// The below functions are called when right after the input file has been read in order to fill the memory to prepare for execution
// mret_t merry_memory_load(MerryMemory *memory, mqptr_t to_load, msize_t num_of_qs);

/*
// here number of qs means how many qwords to read
// this will be specially helpful during operations where single instruction operates on multiple data
// mret_t merry_memory_read_chunk(MerryMemory *memory, maddress_t address, mqptr_t _store_in, msize_t _num_of_qs);
mret_t merry_memory_write_chunk(MerryMemory *memory, maddress_t address, mqptr_t _to_write, msize_t _num_of_qs_to_write);
*/

#endif