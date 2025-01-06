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
