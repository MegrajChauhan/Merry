#ifndef _MERRY_
#define _MERRY_

#include <stdio.h>
#include <stdlib.h>
#include "merry_os.h"

#define _MERRY_MAX_OPTIONS_ 3 // I mean this is given. The person can't ask anything to move both forward and backward at the same time. But this is actually not representing the limit
                              // this represents the number of options

typedef enum MerryCLOption_t MerryCLOption_t;
typedef struct MerryCLOption MerryCLOption; // an option
typedef struct MerryCLP MerryCLP;           // command line parser

enum MerryCLOption_t
{
    _OPT_HELP, // '-h; , '--h', '-help', '--help'
    _OPT_FILE, // -I <Input file>
    _OPT_VER,  // -v, --v, -version, --version
};

struct MerryCLOption
{
    union
    {
        mstr_t *_given_value_str_;
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