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
#include <merry_os.h>
#include "merry_helpers.h"

#define _MERRY_MAX_OPTIONS_ 4

typedef enum MerryCLOption_t MerryCLOption_t;
typedef struct MerryCLOption MerryCLOption; // an option
typedef struct MerryCLP MerryCLP;           // command line parser

enum MerryCLOption_t
{
    _OPT_HELP,      // '-h; , '--h', '-help', '--help'
    _OPT_FILE,      // -I <Input file>
    _OPT_VER,       // -v, --v, -version, --version
    _OPT_DUMP_FILE, // --dump-file, -df
    _OPT_CLO,       // --
    _OPT_FREEZE,    // -F, --freeze
};

struct MerryCLP
{
    mbool_t _help;
    mbool_t _version;
    mbool_t freeze;
    mstr_t _inp_file;
    mstr_t _dump_file;
    mbool_t _dump;
    char **_options_;
    msize_t option_count;
};

MerryCLP *merry_parse_options(int argc, char **argv);

void merry_print_help();

void merry_destroy_parser(MerryCLP *clp);

mret_t merry_parse_d_options(MerryCLP *clp, char *opt);

#endif