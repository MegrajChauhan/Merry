/*
 * Temporary allocator for the Merry VM
 * MIT License
 *
 * Copyright (c) 2023 MegrajChauhan
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
#ifndef _MERRY_MEMALLOC_TEMP_
#define _MERRY_MEMALLOC_TEMP_

#include "../../utils/merry_config.h"
#include "../../utils/merry_types.h"
#include "../thread/merry_thread.h" // allocators are thread safe

#if defined(_MERRY_HOST_OS_LINUX_)
#include "merry_mem_unix.h"
#define _MERRY_MEMTP_UNIX_ 1
#endif

#define _MERRY_ALLOC_MAGIC_NUM_ 0xFFFFFFFFFFFFFFF8

_MERRY_ALWAYS_INLINE msize_t merry_align_size(msize_t size)
{
    return (size + 7) & _MERRY_ALLOC_MAGIC_NUM_;
}

/*We need sort of like sections or departments here to handle different tasks.*/
/*One section needs to get memory from the OS, keep track of the memory we have and one needs to allocate that memory per request*/
/*The other section also has to keep track of free blocks, allocated blocks and make sure that everything works out.*/

/*Overseer*/
static mbptr_t merry_temp_start_address = NULL; // the starting address of the memory we will manage
static mbptr_t merry_temp_end_address = NULL;   // the ending address of the memory we will manage
static msize_t merry_temp_managed_size = 0;     // the size of memory managed by temporary allocator
static msize_t merry_temp_memory_in_use = 0;    // obtained by [managed_size - (current_pos - start_address)]

/*Initialize the memory pool by size bytes by requesting the OS*/
void *merry_temp_overseer_alloc(msize_t size);
/*Free whatever memory was managed by the Overseer*/
void merry_temp_overseer_free();
/*Increase the memory pool size managed by the Overseer[Should be hardly used]*/
mret_t merry_temp_overseer_increase_pool_size(msize_t inc_size);

/*Allocator*/

typedef struct MerryTempAllocator MerryTempAllocator;
typedef struct MerryTempAllocBlock MerryTempAllocBlock; // the memory block

struct MerryTempAllocBlock
{
    // first bit of this field will be used to check if the block is free or not
    msize_t block_len; // number of bytes of this block[This is aligned to make memory access faster]
    // for making the linked list
    MerryTempAllocBlock *next;
    MerryTempAllocBlock *prev;
};

#define _MERRY_TEMP_ALLOC_BLOCK_SIZE_ (sizeof(MerryTempAllocBlock))
#define _MERRY_TEMP_GET_FREE_MEMSIZE_ (merry_temp_managed_size - merry_temp_memory_in_use)
#define _MERRY_TEMP_GET_CURRENT_POS_ (merry_temp_start_address + merry_temp_memory_in_use)
#define _MERRY_TEMP_UPDATE_MEM_USE_SIZE_(__add) merry_temp_memory_in_use += __add + _MERRY_TEMP_ALLOC_BLOCK_SIZE_
#define _MERRY_TEMP_GET_INUSE_FLAG_(block) (block->block_len >> 63)

// temporary allocator can use the Global declared variables of overseer for most of the information but use the functions to modify them
// the allocator is required to update memory use
struct MerryTempAllocator
{
    MerryTempAllocBlock *free_list;      // the free list
    MerryTempAllocBlock *allocated_list; // the allocated list
    MerryMutex *lock;                    // the allocator is thread safe
};

// the thread abstraction library is the only library that uses malloc for memory allocation
static MerryTempAllocator allocator = {.free_list = NULL, .allocated_list = NULL, .lock = NULL}; // the allocator

// while allocating more memory is actually allocated rather than what is requested if requested size is not a multiple of 8
// This ensures fast memory access but wastes a lot of memory which is worth the trade-off
/*This initialization is a must*/
mret_t merry_temp_alloc_init(msize_t init_size);

_MERRY_ALWAYS_INLINE merry_temp_update_alloclist(MerryTempAllocBlock *newblock)
{
    // this will update the allocated_list
    if (allocator.allocated_list == NULL)
        allocator.allocated_list = newblock; // set as the head
}

/*Allocate a block*/
mptr_t merry_temp_alloc(msize_t size);
/*Free the block*/
void merry_temp_free(mptr_t _ptr);

#endif