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
#ifndef _MERRY_FIO_
#define _MERRY_FIO_

#include <merry_types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define _MERRY_OPEN_READ_ O_RDONLY
#define _MERRY_OPEN_WRITE_ O_WRONLY
#define _MERRY_OPEN_READ_WRITE_ O_RDWR
#define _MERRY_OPEN_APPEND_ O_APPEND
#define _MERRY_OPEN_TRUNC_ O_TRUNC // truncate
#define _MERRY_CREATE_NEW_ O_CREAT

// Permissions for when creating a new file
#define _MERRY_PERMIT_READ_ S_IRUSR
#define _MERRY_PERMIT_WRITE_ S_IWUSR

#define _MERRY_MODE_READ_ 0
#define _MERRY_MODE_WRITE 1     // If doesn't exist, create one
#define _MERRY_MODE_APPEND_ 2   // if doesn't exist, create one
#define _MERRY_MODE_TRUNCATE_ 3 // if doesn't exist, create one
#define _MERRY_MODE_READ_WRITE_ 4

/*
 Read as binary and write binary are just simply read and write but there are no intermediate conversions done on the data.
*/

// _fd must not be changed
mret_t merry_open_new_file(mstr_t filepath, msize_t mode, mqptr_t _fd);

void merry_close_file(mqptr_t _fd);

msize_t merry_read_from_file(mqptr_t _fd, mbptr_t _store_in, msize_t number_of_bytes);

msize_t merry_write_to_file(mqptr_t _fd, mbptr_t src, msize_t num_of_bytes);

// Seek to _off from relative_pos(or as said in the docs, "whence")
mret_t merry_seek_in_file(mqptr_t _fd, msize_t _off, msize_t relative_pos);

#endif