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
#ifndef _MERRY_
#define _MERRY_

#include <stdio.h>
#include <stdlib.h>
#include "merry_os.h"

#define _MERRY_MAX_OPTIONS_ 4 // I mean this is given. The person can't ask anything to move both forward and backward at the same time. But this is actually not representing the limit
                              // this represents the number of options

typedef enum MerryCLOption_t MerryCLOption_t;
typedef struct MerryCLOption MerryCLOption; // an option
typedef struct MerryCLP MerryCLP;           // command line parser

enum MerryCLOption_t
{
    _OPT_HELP,          // '-h; , '--h', '-help', '--help'
    _OPT_FILE,          // -I <Input file>
    _OPT_VER,           // -v, --v, -version, --version
    _OPT_ENABLE_LOGGER, // -l [The use of this flag doesn't ensure that the log file will be generated]
                        // the logger may fail to get initialized and enabling the logger slows down the performance of the VM
};

struct MerryCLOption
{
    union
    {
        mstr_t *_given_value_str_;
        // more as needed
    };
    mbool_t provided;
};

struct MerryCLP
{
    MerryCLOption options[_MERRY_MAX_OPTIONS_];
};


MerryCLP *merry_parse_options(int argc, char **argv);

void merry_print_help();

void merry_destroy_parser(MerryCLP *clp);

#endif