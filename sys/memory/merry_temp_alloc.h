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

/*We need sort of like sections or departments here to handle different tasks.*/
/*One section needs to get memory from the OS, keep track of the memory we have and one needs to allocate that memory per request*/
/*The other section also has to keep track of free blocks, allocated blocks and make sure that everything works out.*/

/*Overseer*/
static mbptr_t merry_temp_start_address = NULL; // the starting address of the memory we will manage
static mbptr_t merry_temp_end_address = NULL;   // the ending address of the memory we will manage
static msize_t merry_temp_managed_size = 0;     // the size of memory managed by temporary allocator
static mbptr_t merry_temp_current_pos = NULL;   // points to the first free byte of the managed block[Updated as the block gets used]
static msize_t merry_temp_memory_in_use = 0;    // obtained by [managed_size - (current_pos - start_address)]

/*Initialize the memory pool by size bytes by requesting the OS*/
mret_t merry_temp_overseer_alloc(msize_t size);

#endif