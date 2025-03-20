#include "merry.h"

// State handlers
_MERRY_INTERNAL_ MerryState handle_start(MerryStateMachine *sm);
_MERRY_INTERNAL_ MerryState handle_help(MerryStateMachine *sm);
_MERRY_INTERNAL_ MerryState handle_version(MerryStateMachine *sm);
_MERRY_INTERNAL_ MerryState handle_input_file(MerryStateMachine *sm);
_MERRY_INTERNAL_ MerryState handle_dump_file(MerryStateMachine *sm);
_MERRY_INTERNAL_ MerryState handle_freeze(MerryStateMachine *sm);
_MERRY_INTERNAL_ MerryState handle_unknown(MerryStateMachine *sm);

MerryCLP *merry_parse_options(int argc, char **argv, mbool_t child)
{
    inlog("Parsing command line options...");
    if (argc < 2)
    {
        mreport("Error parsing command-line options: no arguments provided.");
        return NULL;
    }

    // Initialize parser and state machine
    MerryCLP *clp = (MerryCLP *)malloc(sizeof(MerryCLP));
    if (!clp)
    {
        mreport("Failed to allocate memory for command-line parser.");
        return NULL;
    }
    memset(clp, 0, sizeof(MerryCLP));

    MerryStateMachine sm = {
        .current_state = STATE_START,
        .parser = clp,
        .argc = argc,
        .argv = argv,
        .index = 1,
        .is_child = child};

    while (sm.current_state != STATE_DONE)
    {
        switch (sm.current_state)
        {
        case STATE_START:
            sm.current_state = handle_start(&sm);
            break;
        case STATE_HELP:
            sm.current_state = handle_help(&sm);
            break;
        case STATE_VERSION:
            sm.current_state = handle_version(&sm);
            break;
        case STATE_INPUT_FILE:
            sm.current_state = handle_input_file(&sm);
            break;
        case STATE_DUMP_FILE:
            sm.current_state = handle_dump_file(&sm);
            break;
        case STATE_FREEZE:
            sm.current_state = handle_freeze(&sm);
            break;
        case STATE_UNKNOWN:
            sm.current_state = handle_unknown(&sm);
            break;
        default:
            sm.current_state = STATE_DONE;
            break;
        }
    }

    return clp;
}

static MerryState handle_start(MerryStateMachine *sm)
{
    if (sm->index >= sm->argc)
        return STATE_DONE;

    char *arg = sm->argv[sm->index];
    sm->index++;

    if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
    {
        return STATE_HELP;
    }
    else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0)
    {
        return STATE_VERSION;
    }
    else if (strcmp(arg, "-f") == 0)
    {
        return STATE_INPUT_FILE;
    }
    else if (strncmp(arg, "--dump-file", 11) == 0 || strncmp(arg, "-df", 3) == 0)
    {
        return STATE_DUMP_FILE;
    }
    else if (strcmp(arg, "-F") == 0 || strcmp(arg, "--freeze") == 0)
    {
        return STATE_FREEZE;
    }
    else if (strcmp(arg, "--") == 0)
    {
        sm->parser->options = &sm->argv[sm->index];
        sm->parser->option_count = sm->argc - sm->index;
        return STATE_DONE;
    }
    else
    {
        return STATE_UNKNOWN;
    }
}

static MerryState handle_help(MerryStateMachine *sm)
{
    sm->parser->help = mtrue;
    return STATE_DONE;
}

static MerryState handle_version(MerryStateMachine *sm)
{
    sm->parser->version = mtrue;
    return STATE_DONE;
}

static MerryState handle_input_file(MerryStateMachine *sm)
{
    if (sm->index >= sm->argc)
    {
        mreport("Expected input file after '-f' option.");
        sm->parser->help = mtrue;
        return STATE_DONE;
    }

    sm->parser->inp_file = sm->argv[sm->index];
    sm->index++;
    return STATE_START;
}

static MerryState handle_dump_file(MerryStateMachine *sm)
{
    char *arg = sm->argv[sm->index - 1];

    char *path = strchr(arg, '=');
    if (!path || strlen(path) <= 1)
    {
        mreport("Expected file path for dump file option.");
        sm->parser->help = mtrue;
        return STATE_DONE;
    }

    sm->parser->dump_file = path + 1;
    sm->parser->dump = mtrue;
    return STATE_START;
}

static MerryState handle_freeze(MerryStateMachine *sm)
{
    sm->parser->freeze = mtrue;
    return STATE_START;
}

static MerryState handle_unknown(MerryStateMachine *sm)
{
    mreportA("Unknown option '%s'\n", sm->argv[sm->index - 1]);
    sm->parser->help = mtrue;
    return STATE_DONE;
}

void merry_print_help()
{
    printf(
        "Usage: merry -f <path_to_input_file> [OPTIONS]\n"
        "OPTIONS:\n"
        "-h, --help                      Print this help\n"
        "-v, --version                   Display the version\n"
        "-f <file>                       Specify input file\n"
        "--dump-file=<path>, -df=<path>  Generate a dump file\n"
        "-F, --freeze                    Enable freeze mode\n"
        "--                              Specify options for your program\n");
}

void merry_destroy_parser(MerryCLP *clp)
{
    if (clp)
        free(clp);
}
