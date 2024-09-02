#include "merry.h"

MerryCLP *merry_parse_options(int argc, char **argv)
{
    if (argc < 2)
    {
        // we have an error here
        rlog("Error parsing command line options.\n", NULL);
        return RET_NULL;
    }
    MerryCLP *clp = (MerryCLP *)malloc(sizeof(MerryCLP));
    if (clp == NULL)
    {
        // This is funny and unintended.
        // if the malloc fails, this message will be thrown while displaying the "help" message
        // even though the command was probably correct
        rlog("Failed to parse options\n", NULL);
        return RET_NULL;
    }
    clp->_dump = mfalse;
    clp->option_count = 0;
    clp->_dump_file = NULL;
    clp->_inp_file = NULL;
    clp->_help = mfalse;
    clp->_options_ = NULL;
    clp->_version = mfalse;
    clp->freeze = mfalse;
    mbool_t _clo_provided_ = mfalse;
    int i = 1;
    for (; i < argc; i++)
    {
        if (_clo_provided_ == mtrue)
            break;
        switch (argv[i][0])
        {
        case '-':
        {
            // some options may start with '-' and some may start with '--'
            // there is no clear distinction for it. For eg: '-h', '--help', '--h' and '-help' are the same and valid
            switch (argv[i][1])
            {
            case '-':
            {
                switch (argv[i][2])
                {
                case 'h':
                    if (strcmp(&argv[i][2], "help") == 0 || strcmp(&argv[i][2], "h") == 0)
                    {
                        clp->_help = mtrue; // the program asks for help
                    }
                    break;
                case 'v':
                    if (strcmp(&argv[i][2], "version") == 0 || strcmp(&argv[i][2], "v") == 0)
                    {
                        clp->_version = mtrue; // the program asks for version
                    }
                    break;
                case 'd':
                    if (merry_parse_d_options(clp, argv[i]) == RET_FAILURE)
                    {
                        clp->_help = mtrue;
                        return clp;
                    }
                    break;
                case 'f':
                    if (strcmp(&argv[i][2], "freeze") == 0)
                        clp->freeze = mtrue; // the program asks for version
                    break;
                default:
                    _clo_provided_ = mtrue;
                    break;
                }
                break;
            }
                // we don't have a second '-'
            case 'h':
                if (strcmp(&argv[i][1], "help") == 0 || strcmp(&argv[i][1], "h") == 0)
                {
                    clp->_help = mtrue; // the program asks for help
                }
                break;
            case 'v':
                if (strcmp(&argv[i][1], "v") == 0 || strcmp(&argv[i][1], "version") == 0)
                {
                    clp->_version = mtrue; // the program asks for version
                }
                break;
            case 'f':
                // the following option after this must be a destination to the input file
                if (clp->_inp_file != NULL)
                {
                    rlog("Two input files provided: Ignoring the second one.\n", NULL);
                    break;
                }
                if (argc < (i + 2))
                {
                    rlog("Expected path to input file after '-f' option, got EOF instead.\n", NULL);
                    clp->_help = mtrue;
                    return clp;
                }
                clp->_inp_file = argv[i + 1];
                i++;
                break;
            case 'F':
                clp->freeze = mtrue;
                break;
            case 'd':
                if (merry_parse_d_options(clp, argv[i]) == RET_FAILURE)
                {
                    clp->_help = mtrue;
                    return clp;
                }
                break;
            default:
                rlog("Unknown option '%s'\n", argv[i]);
                clp->_help = mtrue;
                return clp;
            }
            break;
        }
        default:
            rlog("Unknown option '%s'\n", argv[i]);
            clp->_help = mtrue;
            return clp;
        }
    }
    if (_clo_provided_ == mtrue)
    {
        if (argc == (i))
        {
            rlog("Expected Command Line Options after '--' but found none.\n", NULL);
            clp->_help = mtrue;
            return clp;
        }
        clp->_options_ = (char **)malloc(sizeof(char *) * (argc - i));
        if (clp->_options_ == NULL)
        {
            rlog("Failed to fully parse the options.\n", NULL);
            merry_destroy_parser(clp);
            return RET_NULL;
        }
        msize_t j = 0;
        for (; i < argc; i++)
        {
            clp->_options_[j] = argv[i];
            j++;
        }
        clp->option_count = j;
    }
    return clp;
}

void merry_print_help()
{
    log(
        "Usage; merry -f <path_to_input_file> [OPTIONS]\n"
        "OPTIONS:\n"
        "-h, --h, -help, --help        --> Print this help\n"
        "-v, --v, -version, --version  --> Display the current version\n"
        "-f                            --> Provide the path to the input file\n"
        "--                            --> Provide Command Line Options for your Program\n"
        "--dump-file=[PATH],-df=[PATH] --> Tell the VM to produce a dumpfile\n"
        "                                  If '=' is given, a file name is expected otherwise no need.\n"
        "-F, --freeze                  --> If DE flag is set; causes the VM to wait for connection for debugging.\n",
        NULL);
}

void merry_destroy_parser(MerryCLP *clp)
{
    if (clp == NULL)
        return;
    if (clp->_options_ != NULL)
        free(clp->_options_);
    free(clp);
}

mret_t merry_parse_d_options(MerryCLP *clp, char *opt)
{
    msize_t len = strlen(opt);
    if (str_starts_with(opt, "--dump-file") == mtrue)
    {
        if (len == 11)
        {
            // We have no path provided
            clp->_dump = mtrue;
            clp->_dump_file = NULL;
            return RET_SUCCESS;
        }
        if (str_ends_with(opt, "=") == mtrue)
        {
            rlog("Error: The option '--dump-file' requires a path. Don't use '=' for default filename.\n", NULL);
            return RET_FAILURE;
        }
        if (opt[11] != '=')
        {
            rlog("Error: The option '--dump-file' requires a path. Don't use '=' for default filename.\n", NULL);
            return RET_FAILURE;
        }
        clp->_dump_file = opt + 12;
        clp->_dump = mtrue;
        return RET_SUCCESS;
    }
    else if (str_starts_with(opt, "-df") == mtrue)
    {
        if (len == 3)
        {
            // We have no path provided
            clp->_dump = mtrue;
            clp->_dump_file = NULL;
            return RET_SUCCESS;
        }
        if (str_ends_with(opt, "=") == mtrue)
        {
            rlog("Error: The option '-df' requires a path. Don't use '=' for default filename.\n", NULL);
            return RET_FAILURE;
        }
        if (opt[3] != '=')
        {
            rlog("Error: The option '-df' requires a path. Don't use '=' for default filename.\n", NULL);
            return RET_FAILURE;
        }
        clp->_dump_file = opt + 4;
        clp->_dump = mtrue;
        return RET_SUCCESS;
    }
    return RET_FAILURE;
}