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

#include <unistd.h>
#include <sys/mman.h>
#include "../../../sys/thread/merry_thread.h"

#define _MERRY_ALLOC_MAGIC_NUM_ 0xFFFFFFFFFFFFFFF8

_MERRY_ALWAYS_INLINE msize_t merry_align_size(msize_t size)
{
    return (size + 7) & _MERRY_ALLOC_MAGIC_NUM_;
}

// For buffers, we cannot use this at all or we may need to modify this

// these are abstractions for getting more memory
// for each host, we can use this abstraction to follow their conventions
#define _MERRY_INC_MEMLIM_(size) sbrk(size)
#define _MERRY_GET_CURRENT_BRK_POINT_ _MERRY_INC_MEMLIM_(0)

static mbptr_t merry_allocator_start_address = NULL; // the starting address of the memory we will manage
static mbptr_t merry_allocator_end_address = NULL;   // the ending address of the memory we will manage
static msize_t merry_allocator_managed_size = 0;     // the size of memory managed by temporary allocator
static msize_t merry_allocator_memory_in_use = 0;    // obtained by [managed_size - (current_pos - start_address)]

/*Initialize the memory pool by size bytes by requesting the OS*/
mptr_t merry_allocator_overseer_alloc(msize_t size);
/*Free whatever memory was managed by the Overseer*/
void merry_allocator_overseer_free();
/*Increase the memory pool size managed by the Overseer[Should be hardly used]*/
mret_t merry_allocator_overseer_increase_pool_size(msize_t inc_size);

/*Allocator*/

typedef struct MerryAllocator MerryAllocator;
typedef struct MerryAllocatorBlock MerryAllocatorBlock; // the memory block

struct MerryAllocatorBlock
{
    // first bit of this field will be used to check if the block is free or not
    msize_t block_len; // number of bytes of this block[This is aligned to make memory access faster]
    // for making the linked list
    MerryAllocatorBlock *next;
    MerryAllocatorBlock *prev;
};

#define _MERRY_ALLOCATOR_ALLOC_BLOCK_SIZE_ (sizeof(MerryAllocatorBlock))
#define _MERRY_ALLOCATOR_GET_FREE_MEMSIZE_ (merry_allocator_managed_size - merry_allocator_memory_in_use)
#define _MERRY_ALLOCATOR_GET_CURRENT_POS_ (merry_allocator_start_address + merry_allocator_memory_in_use)
#define _MERRY_ALLOCATOR_UPDATE_MEM_USE_SIZE_(__add) merry_allocator_memory_in_use += __add + _MERRY_ALLOCATOR_ALLOC_BLOCK_SIZE_
#define _MERRY_ALLOCATOR_ARE_BLOCKSADJ_(block1) (((mbptr_t)block1 + _MERRY_ALLOCATOR_ALLOC_BLOCK_SIZE_ + block1->block_len) == (mbptr_t)block1->next)

#define _MERRY_ALLOCATOR_SET_EMPTY_HEAD_(head) \
    {                                          \
        (head)->next = (head);                 \
        (head)->prev = (head);                 \
    }

#define _MERRY_ALLOCATOR_SET_ALLOC_EMPTY_HEAD_(head) \
    {                                                \
        _MERRY_ALLOCATOR_SET_EMPTY_HEAD_(head)       \
        allocator.allocated_list = head;             \
    }

#define _MERRY_ALLOCATOR_SET_FREE_EMPTY_HEAD_(head) \
    {                                               \
        _MERRY_ALLOCATOR_SET_EMPTY_HEAD_(head)      \
        allocator.free_list = head;                 \
    }

#define _MERRY_ALLOCATOR_ADD_TAIL_ALLOC_(new_tail)                      \
    {                                                                   \
        MerryAllocatorBlock *old_tail = allocator.allocated_list->prev; \
        old_tail->next = new_tail;                                      \
        new_tail->prev = old_tail;                                      \
        new_tail->next = allocator.allocated_list;                      \
        allocator.allocated_list->prev = new_tail;                      \
    }

#define _MERRY_ALLOCATOR_ADD_TAIL_FREE_(new_tail)                  \
    {                                                              \
        MerryAllocatorBlock *old_tail = allocator.free_list->prev; \
        old_tail->next = new_tail;                                 \
        new_tail->prev = old_tail;                                 \
        new_tail->next = allocator.free_list;                      \
        allocator.free_list->prev = new_tail;                      \
    }

// temporary allocator can use the Global declared variables of overseer for most of the information but use the functions to modify them
// the allocator is required to update memory use
struct MerryAllocator
{
    MerryAllocatorBlock *free_list;      // the free list
    MerryAllocatorBlock *allocated_list; // the allocated list
    MerryMutex *lock;                    // the allocator is thread safe
};

// the thread abstraction library is the only library that uses malloc for memory allocation
static MerryAllocator allocator = {.free_list = NULL, .allocated_list = NULL, .lock = NULL}; // the allocator

// while allocating more memory is actually allocated rather than what is requested if requested size is not a multiple of 8
// This ensures fast memory access but wastes a lot of memory which is worth the trade-off

mret_t merry_allocator_alloc_init(msize_t init_size);

/*free the allocator*/
void merry_allocator_alloc_free();

/*Allocate a block*/
mptr_t merry_allocator_alloc(msize_t size);
/*Free the block*/
void merry_allocator_free(mptr_t _ptr);
/*reallocate a block*/
mptr_t merry_allocator_realloc(mptr_t _old_ptr, msize_t new_size);

#endif