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

// #include "../lib/include/merry_memory_allocator.h" <LEGACY>
#include "../../utils/merry_utils.h"
#include "merry_memory.h"
#include <stdio.h>  // for FILE
#include <string.h> // for string manipulation
#include <stdlib.h> // for conversion from strings to numbers

typedef struct MerryInpFile MerryInpFile;

#define _READ_ERROR_(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#define _READ_DIRERROR_(message, ...) fprintf(stderr, message, ##__VA_ARGS__)
#define _INP_FILE_ORDERING_LITTLE_ _MERRY_LITTLE_ENDIAN_
#define _INP_FILE_ORDERING_BIG_ _MERRY_BIG_ENDIAN_

#define _READER_HEADER_LEN_ 32
// #define _READER_GET_SIGNATURE_(header) (header >> 40)
// #define _READER_GET_SDT_OFF_(header) header & 0xFFFFFFFF
// #define _READER_GET_BYTE_ORDER_(header) (header >> 32) & 0x1

// many of these prints are useless and the unnecessary ones will be removed
// #define read_unexpected_eof() _READ_DIRERROR_("Read Error: Unexpected EOF when an attribute was expected.\n")
// #define read_unexpected_eof_when(attribute) _READ_ERROR_("Read Error: Unexpected EOF when attribute '%s' was expected.\n", attribute)
// #define read_expected_got(expected_attr, got) _READ_ERROR_("Read Error: Unexpected '%s' when attribute '%s' was expected.\n", got, expected_attr)
// #define read_expected(expected_attr) _READ_ERROR_("Read Error: Exptected attribute '%s' but got something else instead.\n", expected_attr)
// #define read_double_attr_provided(attr_prov) _READ_ERROR_("Read Error: The attribute %s was provided multiple times.\n", attr_prov)

#define read_internal_error(msg) _READ_ERROR_("Read Internal Error: %s.\n", msg)
#define read_msg(message, ...) fprintf(stderr, message, __VA_ARGS__)
// num is an array of unsigned characters here
// #define reader_invert_byte_order(num, k) 

// the main purpose here is to read the binary input file and then convert it into something the VM can understand
// The extension for the input file is ".mbin" for "merry binary/bin"

struct MerryInpFile
{
    mcstr_t _file_name; // the input file's name
    // mstr_t _file_contents; // the file's contents
    msize_t file_len; // the number of bytes in the file
    FILE *f;          // the opened file
    // the metadata representation
    msize_t byte_order;
    msize_t dlen; // data bytes len
    msize_t ilen; // the instruction bytes len
    msize_t slen; // the string len
    // the file results
    msize_t ipage_count;
    msize_t dpage_count;
    mqptr_t *_data;         // the read data
    mqptr_t *_instructions; // the read instructions
};

MerryInpFile *merry_read_file(mcstr_t _file_name);

void merry_destory_reader(MerryInpFile *inp);

#endif