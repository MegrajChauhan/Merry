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
#include <string.h>
#include <merry_os.h>
#include "merry_helpers.h"
#include "merry_console.h"

#define _MERRY_MAX_OPTIONS_ 4

typedef enum MerryState MerryState;
typedef struct MerryCLP MerryCLP;
typedef struct MerryStateMachine MerryStateMachine;

enum MerryState
{
    STATE_START,
    STATE_HELP,
    STATE_VERSION,
    STATE_INPUT_FILE,
    STATE_DUMP_FILE,
    STATE_FREEZE,
    STATE_PARSE_OPTIONS,
    STATE_UNKNOWN,
    STATE_DONE,
};

struct MerryCLP
{
    mbool_t help;
    mbool_t version;
    mbool_t freeze;
    mstr_t inp_file;
    mstr_t dump_file;
    mbool_t dump;
    char **options;
    msize_t option_count;
    mbool_t is_child;
    mstr_t entry;
};

struct MerryStateMachine
{
    MerryState current_state;
    MerryCLP *parser;
    int argc;
    char **argv;
    int index; // Current index in argv
    mbool_t is_child;
};

MerryCLP *merry_parse_options(int argc, char **argv, mbool_t child);

void merry_print_help();

void merry_destroy_parser(MerryCLP *clp);

#endif
