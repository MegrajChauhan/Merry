/*
 * Configuration for the Merry VM
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
#ifndef _MERRY_INPUT_
#define _MERRY_INPUT_

#if defined(_WIN64)
#include "..\..\..\utils\merry_types.h"
#include <stdio.h>
#include <stdlib.h>
#include "..\merry_dmemory.h"
#else
#include "../../../utils/merry_types.h"
#include <stdio.h>
#include <stdlib.h>
#include "../merry_dmemory.h"
#endif


// #define _MERRY_BASE_LEN_ 32

// the simplest one
mret_t merry_read_char(MerryDMemory *mem, maddress_t address); // _store_in is an address in the data_mem that the manager will provide

// // implementing the above is all that is needed
// // reading integers and strings can be implemented with the above as the base

// // This function is for this module specifically but could be useful in the future
// msize_t merry_readline(mstr_t _num, msize_t len); /*Read until newline, allocate necessary memory as needed if len is not enough*/

// // this is going to read a number of 2 bytes long
// void merry_read_word(mptr_t _store_in);

// // this is going to read a number of 4 bytes long
// void merry_read_dword(mptr_t _store_in);

// // this is going to read a number of 8 bytes long
// void merry_read_qword(mptr_t _store_in);

#endif