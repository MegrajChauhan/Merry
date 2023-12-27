/*
 * Memory communication abstraction for the Merry VM
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
#ifndef _MERRY_MEMORYABS_UNIX_
#define _MERRY_MEMORYABS_UNIX_

#include <unistd.h>
#include <sys/mman.h>
#include "../thread/merry_thread.h"

// For buffers, we cannot use this at all or we may need to modify this

// these are abstractions for getting more memory
// for each host, we can use this abstraction to follow their conventions 
#define _MERRY_INC_MEMLIM_(size) sbrk(size)
#define _MERRY_MAP_MEM_(size) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0)
#define _MERRY_UNMAP_MEM_(mem, size) munmap(mem, size)

#define _MERRY_GET_CURRENT_BRK_POINT_ _MERRY_INC_MEMLIM_(0)

#endif
