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
#include <stdio.h> // remove this

#define _MERRY_ALLOC_MAGIC_NUM_ 0xFFFFFFFFFFFFFFF8
#define _MERRY_ALLOC_PAGE_LEN_ 4096

// based on what Merry requires we will keep track of how many bytes of allocation is requested
// for now we are going to assume that Merry is going to make a request for 8 bytes[for pointers], 16 bytes, 32 bytes, 64 bytes and 128 bytes at most
// of course the allocator will be handling other sizes as well but they will be "misc" requests.
// Even Merry's memory will need the allocator to be able to handle requests of about 1MB long which it should be able to do.

_MERRY_ALWAYS_INLINE msize_t merry_align_size(msize_t size)
{
    return (size + 7) & _MERRY_ALLOC_MAGIC_NUM_;
}

// We only need this allocator to fulfill the requests from Merry
// we only allocate aligned memory
// we don't care about anything else such as colored memory or whatever they are called
// Based on the size of the request, we will decide on what to use: sbrk or mmap

typedef struct MerryAllocBlock MerryAllocBlock;
typedef struct MerryAllocPage MerryAllocPage;
typedef struct MerryAllocator MerryAllocator;

// the array of blocks is linear and not circular
struct MerryAllocBlock
{
    msize_t _block_size;         // the size of the memory that this block is holding on to
    MerryAllocBlock *next;       // the next block
    MerryAllocBlock *prev;       // the previous block
    MerryAllocPage *parent_page; // the page to which this block belongs to
};

struct MerryAllocPage
{
    mptr_t _start_address;           // the starting address of this alloc page
    MerryAllocPage *next_page;       // the next page
    MerryAllocBlock *entry_non_free; // the first allocated block in the alloc page
    MerryAllocBlock *entry_free;     // the first free block in this alloc page
    msize_t _is_mapped;
    unsigned int _remaining_page; // after allocating the memory how much of the page's size is left
    unsigned int _used_size;      // how much has been allocated
};

struct MerryAllocator
{
    MerryMutex lock;         // the allocator is thread safe
    MerryAllocPage *pg_8;    // the page for 8 bytes memory chunk
    MerryAllocPage *pg_16;   // the page for 16 bytes memory chunk
    MerryAllocPage *pg_32;   // the page for 32 bytes memory chunk
    MerryAllocPage *pg_64;   // the page for 64 bytes memory chunk
    MerryAllocPage *pg_misc; // the page for misc bytes memory chunk
};

#define _MERRY_ALLOC_BLOCK_SIZE_ (sizeof(MerryAllocBlock))
#define _MERRY_ALLOCPG_SIZE_ (sizeof(MerryAllocPage))

#define _MERRY_ALLOC_MAP_PAGE_ _MERRY_MEM_GET_PAGE_(_MERRY_ALLOC_PAGE_LEN_ + _MERRY_ALLOCPG_SIZE_, _MERRY_PROT_DEFAULT_, _MERRY_FLAG_DEFAULT_)
#define _MERRY_ALLOC_UNMAP_PAGE_(address) _MERRY_MEM_GIVE_PAGE_(address, _MERRY_ALLOC_PAGE_LEN_ + _MERRY_ALLOCPG_SIZE_)

/*
 If a request comes for 8 bytes and we have no 8 byte block free for allocation, we will not merge blocks of other pages or split their blocks.
 Instead we will simply allocate another block that will be able to fulfill other 8 bytes request.
 This way the allocations might be fast but the memory footprint of the VM will be huge.
*/
static MerryAllocator allocator;

// we will iniitalize each page and be ready for allocation requests right away.
mret_t merry_allocator_init();

void merry_allocator_destroy();

mptr_t merry_malloc(msize_t size);

void merry_free(mptr_t _ptr);

#endif