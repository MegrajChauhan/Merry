/*
 * The input file reader for the Merry VM
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
#ifndef _MERRY_READER_
#define _MERRY_READER_

#include "../../utils/merry_types.h"
#include "../lib/include/merry_memory_allocator.h"
#include <stdio.h>  // for FILE
#include <string.h> // for conversion from strings to numbers

#define _READ_ERROR_(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#define _FMT_HEX_ 0
#define _FMT_BIN_ 1

// the main purpose here is to read the binary input file and then convert it into something the VM can understand
// The extension for the input file is ".min" for "merry binary/bin"
// the structure of the file is given as
/*
All lines starting with a '.' is some metadata given to the reader.
These metadata must be given at the beginning of the file.
We need to know if the file is in hex or binary format
This is the very first line expected in the input file
.fmt_hex or .fmt_bin
only hexadecimal and binary format is allowed.
Some of these metadata is a must or else unexpected things might happen
Here are all of the metadata available
.dlen [len] and .ilen [len] : Specifies the length of the data section and ilen in bytes. The reader will expect to read len number of bytes as specified.
                              If not specified then no data or instruction is read.
.ibstart [pos] and .ibend [pos]: Specifies the starting bytes position of the instruction bytes and the ending byte position. Starts from 0.
.dbstart [pos] and .dbend [pos]: Specifies the same as above but for data bytes.
Note: The position of data bytes and instruction bytes must not overlap.
*/

typedef struct MerryInpFile MerryInpFile;

struct MerryInpFile
{
    mcstr_t _file_name; // the input file's name
    msize_t curr_line;  // the current line being read
    FILE *f;            // the opened file
    // the metadata representation
    unsigned int _inp_fmt; // the input bytes format
    msize_t dlen;          // data bytes len
    msize_t ilen;          // the instruction bytes len
    msize_t ibstart;       // the instruction bytes starting position
    msize_t ibend;         // instruction bytes ending position
    msize_t dbstart;       // data byte starting position
    msize_t dbend;         // data byte ending position
    // the file results
    mqptr_t _data;         // the read data
    mqptr_t _instructions; // the read instructions
};

#endif