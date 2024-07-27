/*
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
#ifndef _MERRY_MEM_
#define _MERRY_MEM_

#include <sys/mman.h> // for mmap
#include <unistd.h>   // for sbrk
// we use mapping for mostly memory allocation and not for mapping any actual file
// hence we simply need these protection flags and nothing else
// but for future simplicity
#define _MERRY_MEM_GET_PAGE_(size, prot, flags) mmap((void *)-1, size, PROT_READ | PROT_WRITE | prot, MAP_ANONYMOUS | MAP_PRIVATE | flags, -1, 0)
#define _MERRY_MEM_GIVE_PAGE_(addr, size) munmap(addr, size)
#define _MERRY_RET_GET_ERROR_ MAP_FAILED // the error sent by mmap on failure
#define _MERRY_RET_GIVE_ERROR_ -1        // the error sent by munmap on failure
#define _MERRY_PROT_DEFAULT_ 0x00        // default protection flag
#define _MERRY_FLAG_DEFAULT_ 0x00        // default flag

// for extending system break point
// some systems do not provide such functionality(Or Maybe I am just not knowledgeable)
#define _MERRY_MEM_BRK_SUPPORT_ 1                     // meddling with the program's break point is supported
#define _MERRY_MEM_GET_CURRENT_BRK_POINT_ sbrk(0)     // get the current break point
#define _MERRY_MEM_INCREASE_BRK_POINT_(len) sbrk(len) // increase the program's break point by len bytes
#define _MERRY_MEM_BRK_ERROR_ (void *)-1

#endif