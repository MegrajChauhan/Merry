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
#include <string.h> // for string manipulation
#include <stdlib.h> // for conversion from strings to numbers

#define _READ_ERROR_(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#define _READ_DIRERROR_(message) fprintf(stderr, message)
#define _FMT_HEX_ 0
#define _FMT_BIN_ 1

// many of these prints are useless and the unnecessary ones will be removed
#define read_unexpected_eof() _READ_DIRERROR_("Read Error: Unexpected EOF when an attribute was expected.\n")
#define read_unexpected_eof_when(attribute) _READ_ERROR_("Read Error: Unexpected EOF when attribute '%s' was expected.\n", attribute)
#define read_expected_got(expected_attr, got) _READ_ERROR_("Read Error: Unexpected '%s' when attribute '%s' was expected.\n", got, expected_attr)
#define read_expected(expected_attr) _READ_ERROR_("Read Error: Exptected attribute '%s' but got something else instead.\n", expected_attr)
#define read_double_attr_provided(attr_prov) _READ_ERROR_("Read Error: The attribute %s was provided multiple times.\n", attr_prov)
#define read_internal_error() _READ_DIRERROR_("Read Internal Error: Unable to read attributes and properties.\n")
#define read_msg(message, ...) fprintf(stderr, message, __VA_ARGS__)

// the main purpose here is to read the binary input file and then convert it into something the VM can understand
// The extension for the input file is ".mbin" for "merry binary/bin"

typedef struct MerryInpFile MerryInpFile;

struct MerryInpFile
{
    mcstr_t _file_name;    // the input file's name
    mstr_t _file_contents; // the file's contents
    msize_t file_len;
    mstr_t iter;           // the file's iterator
    FILE *f;               // the opened file
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

MerryInpFile *merry_read_file(mcstr_t _file_name);

void merry_destory_reader(MerryInpFile *inp);

#endif