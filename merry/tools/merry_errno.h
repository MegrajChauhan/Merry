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

#include <errno.h>
#include <merry_types.h>
#include <merry_utils.h>
#include <merry_console.h>

/*
Since each platform has different number of errno values, we have to be really careful to deal with them.
*/
#define MERRY_EACCES 0         // Permission denied
#define MERRY_EEXIST 1         // File exists
#define MERRY_EFAULT 2         // Bad address
#define MERRY_EINTR 3          // Interrupted function call
#define MERRY_EINVAL 4         // Invalid argument
#define MERRY_EIO 5            // Input/output error
#define MERRY_EISDIR 6         // Is a directory
#define MERRY_EMFILE 7         // Too many open files
#define MERRY_ENAMETOOLONG 8   // File name too long
#define MERRY_ENFILE 9         // Too many open files in system
#define MERRY_ENODEV 10        // No such device
#define MERRY_ENOENT 11        // No such file or directory
#define MERRY_ENOMEM 12        // Not enough space
#define MERRY_ENOSPC 13        // No space left on device
#define MERRY_ENOTDIR 14       // Not a directory
#define MERRY_ENXIO 15         // No such device or address
#define MERRY_EOVERFLOW 16     // Value too large to be stored in data type
#define MERRY_EROFS 17         // Read-only file system
#define MERRY_ETIMEDOUT 18     // Connection timed out
#define MERRY_ECONNREFUSED 19  // Connection refused
#define MERRY_EHOSTUNREACH 20  // No route to host
#define MERRY_EADDRINUSE 21    // Address already in use
#define MERRY_EADDRNOTAVAIL 22 // Address not available
#define MERRY_EAFNOSUPPORT 23  // Address family not supported
#define MERRY_ENOTCONN 24      // Socket is not connected
#define MERRY_ECONNRESET 25    // Connection reset
#define MERRY_EPIPE 26         // Broken pipe
#define MERRY_EBADF 27         // Bad file descriptor
#define MERRY_ELOOP 28         // Too many levels of symbolic links
#define MERRY_ENOTEMPTY 29     // Directory not empty
#define MERRY_EPERM 30         // Operation not permitted
#define MERRY_ESHUTDOWN 31     // Cannot send after transport endpoint shutdown

// Our errno value
_MERRY_INTERNAL_ msize_t merrno = (msize_t)-1;

void merry_update_errno();

// This will reset errno
msize_t merry_get_errno();

// This will also reset errno
void merry_interpret_errno();

#endif