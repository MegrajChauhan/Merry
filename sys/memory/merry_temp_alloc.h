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
#define _MERRY_TEMP_ARE_BLOCKSADJ_(block1) ((block1 + _MERRY_TEMP_ALLOC_BLOCK_SIZE_ + block1->block_len) == block1->next)

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
/*free the allocator*/
void merry_temp_alloc_free();

_MERRY_ALWAYS_INLINE void merry_temp_update_alloclist(MerryTempAllocBlock *newblock)
{
    // this will update the allocated_list
    if (allocator.allocated_list == NULL)
    {
        // since the list is circular meaning the block itself is the tail and the head
        newblock->next = newblock;
        newblock->prev = newblock;
        allocator.allocated_list = newblock; // set as the head
        return;
    }
    // we have to set this as the new tail
    MerryTempAllocBlock *old_tail = allocator.allocated_list->prev; // the old tail
    old_tail->next = newblock;                                      // the new tail
    newblock->prev = old_tail;                                      // point to the older tail
    newblock->next = allocator.allocated_list;                      // close the loop
    allocator.allocated_list->prev = newblock;                      // point to the new tail
}

_MERRY_ALWAYS_INLINE void merry_temp_update_freelist(MerryTempAllocBlock *newblock)
{
    // this will update the free list
    if (allocator.free_list == NULL)
    {
        // since the list is circular meaning the block itself is the tail and the head
        newblock->next = newblock;
        newblock->prev = newblock;
        allocator.free_list = newblock; // set as the head
        return;
    }
    // we have to set this as the new tail
    MerryTempAllocBlock *old_tail = allocator.free_list->prev; // the old tail
    old_tail->next = newblock;                                 // the new tail
    newblock->prev = old_tail;                                 // point to the older tail
    newblock->next = allocator.free_list;                      // close the loop
    allocator.free_list->prev = newblock;                      // point to the new tail
}

_MERRY_ALWAYS_INLINE MerryTempAllocBlock *merry_temp_get_first_fit(msize_t size)
{
    // find the block that has size bytes
    // since this function is called when it is confirmed that the free list is not empty, we have nothing to worry about
    MerryTempAllocBlock *temp = allocator.free_list;       // get the head
    MerryTempAllocBlock *curr = allocator.free_list->next; // we loop until curr != temp
    if (temp->block_len >= size)
        return temp;
    while (curr != temp)
    {
        if (curr->block_len >= size)
            return curr;   // we have found it!
        curr = curr->next; // update!
    }
    return RET_NULL; // we found none
}

_MERRY_ALWAYS_INLINE MerryTempAllocBlock *merry_temp_get_adjacent_free_blocks(msize_t size)
{
    // this will look for two adjacent memory blocks and then see if their combined size meets the requirement
    // we also have to put the metadata size in consideration
    MerryTempAllocBlock *temp = allocator.free_list;
    MerryTempAllocBlock *curr = allocator.free_list->next;
    while (curr != temp)
    {
        if (_MERRY_TEMP_ARE_BLOCKSADJ_(curr))
        {
            // if the blocks are adjacent
            if ((curr->block_len + _MERRY_TEMP_ALLOC_BLOCK_SIZE_ + curr->next->block_len) >= size)
                return curr; // we found the block we needed
        }
    }
    return RET_NULL; // we found none
}

_MERRY_ALWAYS_INLINE MerryTempAllocBlock *merry_temp_allocate_new_block(msize_t size)
{
    // size is aligned
    if (_MERRY_TEMP_GET_FREE_MEMSIZE_ < size)
    {
        // there are no free blocks and we have no memory
        if (merry_temp_overseer_increase_pool_size(size * 4) == RET_FAILURE)
            return RET_NULL; // we got no new blocks
    }
    // we now have memory and can allocate new block
    MerryTempAllocBlock *block = NULL;
    block = (MerryTempAllocBlock *)(_MERRY_TEMP_GET_CURRENT_POS_); // get the new block
    block->block_len = size;                                       // the size of the block[We do not need the in use flag]
    return block;
}

/*Allocate a block*/
mptr_t merry_temp_alloc(msize_t size);
/*Free the block*/
void merry_temp_free(mptr_t _ptr);

// helper function

#endif