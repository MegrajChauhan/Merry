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
#ifndef _MERRY_DYNL_
#define _MERRY_DYNL_

#include <merry_types.h>
#include <merry_utils.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef struct MerryDynEntry MerryDynEntry;
typedef struct MerryDynLoader MerryDynLoader;

_MERRY_DEFINE_FUNC_PTR_(mdword_t, dynfunc_t, mptr_t ptr);

struct MerryDynEntry
{
    char entry_name[255]; // the library's name or path
    HINSTANCE lib_handle;
    mbool_t handle_open; // is the library open to use?
};

struct MerryDynLoader
{
    MerryDynEntry *entries; // the entries
    msize_t entry_count;    // number of entries
    msize_t closed_entry_count;
};

static MerryDynLoader loader;

mbool_t merry_loader_init(msize_t initial_entry_count);

void merry_loader_close();

mbool_t merry_loader_loadLib(mstr_t lib_path, msize_t *handle);

void merry_loader_unloadLib(msize_t handle);

// each function in the library must return unsigned int which will be the return value
// the return value of the function is loaded into the Ma register
dynfunc_t merry_loader_getFuncSymbol(msize_t handle, mstr_t sym_name);

#endif