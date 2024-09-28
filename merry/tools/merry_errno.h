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
#ifndef _MERRY_ERRNO_
#define _MERRY_ERRNO_

/*This module isn't being used at all actually.*/

#include <errno.h>
#include <merry_types.h>
#include <merry_utils.h>
#include <merry_console.h>

/*
Since each platform has different number of errno values, we have to be really careful to deal with them.
*/
#define MERRY_NODBG 0       // Debugging was enabled for the child but the debugger wasn't connected at all
#define MERRY_SYSCALLERR 1  // The syscall being handled by the OS didn't expect the provided argument
#define MERRY_DYNERR 2      // The recent dynamic library related request failed
#define MERRY_DYNCLOSED 3   // The handle has already been closed
#define MERRY_INIT_FAILED 4 // Failed to initialize something
#define MERRY_INVAL_ARG 5   // invalid argument
#define MERRY_SYSERR 6      // error that was caused by the host system itself
#define MERRY_VMERR 7  // the error was due to the VM

// Our errno value
_MERRY_INTERNAL_ msize_t merrno = (msize_t)-1;

// This will reset errno
msize_t merry_get_errno(void);

void merry_set_errno(msize_t _err);

#endif