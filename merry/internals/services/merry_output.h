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
#ifndef _MERRY_OUTPUT_
#define _MERRY_OUTPUT_

#if defined(_WIN64)
#include "..\..\..\utils\merry_types.h"
#include <stdio.h>
#include "..\merry_dmemory.h"
#else
#include "../../../utils/merry_types.h"
#include <stdio.h>
#include "../merry_dmemory.h"
#endif


mret_t merry_write_char(MerryDMemory *mem, maddress_t address);

// void merry_write_word(mptr_t _to_write);

// void merry_write_dword(mptr_t _to_write);

// void merry_write_qword(mptr_t _to_write);

// void merry_write_bytes(mptr_t _to_write, msize_t _len);

#endif