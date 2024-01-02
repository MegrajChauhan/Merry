/*
 * Page allocator for the Memory of the Merry VM
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
#ifndef _MERRY_PAGE_ALLOCATOR_
#define _MERRY_PAGE_ALLOCATOR_

// the main job of this module is to abstract away the underlying communication with the OS for memory
// Unlike the memory allocator, page allocator will specifically allocate pages for the Memory by directly mapping large memory portions from the OS.
#include "merry_internals.h"

#if defined(_MERRY_HOST_OS_LINUX_)
#include <sys/mman.h> // for mmap
#define _MERRY_PGALLOC_GET_PAGE_ mmap((void *)-1, _MERRY_MEMORY_ADDRESSES_PER_PAGE_, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)
#define _MERRY_PGALLOC_GIVE_PAGE_(addr) munmap(addr, _MERRY_MEMORY_ADDRESSES_PER_PAGE_)
#define _MERRY_RET_GET_ERROR_ MAP_FAILED
#define _MERRY_RET_GIVE_ERROR_ -1
#endif

// this process can be cumbersome in Windows or similar systems where they refuse to provide the size of the memory we want but instead provide memory that is way
// larger or smaller than what we want but that is a future me problem

mptr_t merry_pgalloc_get_page(void);

mret_t merry_pgalloc_give_page(mptr_t address);

#endif