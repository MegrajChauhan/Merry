/*
 * Types for the Merry VM
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

#ifndef _MERRY_TYPES_
#define _MERRY_TYPES_

#include "merry_config.h"

// boolean types
#define mtrue 1
#define mfalse 0

// for function returns
#define RET_SUCCESS mtrue
#define RET_FAILURE mfalse
#define RET_AMBIGIOUS 2     // extra return to be used for any third sort of return
#define RET_NULL (void *)-1 // used by functions

typedef unsigned char mbool_t; // bool

// these are the memory data types
typedef unsigned char mbyte_t;
typedef unsigned short mword_t;
typedef unsigned int mdword_t;
typedef _MERRY_LONG_ mqword_t;

// some extra definitions
typedef _MERRY_LONG_ maddress_t;
typedef _MERRY_LONG_ msize_t;
typedef mbool_t mret_t; // for function returns

// define function pointers
#define _MERRY_DEFINE_FUNC_PTR_(ret, func_name, ...) typedef ret (*func_name)(__VA_ARGS__);

// pointers for memory types
typedef mbyte_t *mbptr_t;
typedef mword_t *mwptr_t;
typedef mdword_t *mdptr_t;
typedef mqword_t *mqptr_t;

// redifinitions of C types
typedef char *mstr_t;
typedef const char *mcstr_t;
typedef void *mptr_t;

#endif