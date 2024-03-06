#if defined(_WIN64)
#include "internals\merry.h"
#else
#include "internals/merry.h"
#endif

MerryCLP *merry_parse_options(int argc, char **argv)
{
    if (argc < 2)
    {
        // we have an error here
        fprintf(stderr, "Error parsing command line options.\n");
        return RET_NULL;
    }
    MerryCLP *clp = (MerryCLP *)malloc(sizeof(MerryCLP));
    if (clp == NULL)
    {
        // This is funny and unintended.
        // if the malloc fails, this message will be thrown while displaying the "help" message
        // even though the command was probably correct
        fprintf(stderr, "Failed to parse options\n");
        return RET_NULL;
    }
    for (msize_t i = 1; i < argc; i++)
    {
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
                        clp->options[_OPT_HELP].provided = mtrue; // the program asks for help
                    }
                    break;
                case 'v':
                    if (strcmp(&argv[i][2], "version") == 0 || strcmp(&argv[i][2], "v") == 0)
                    {
                        clp->options[_OPT_VER].provided = mtrue; // the program asks for help
                    }
                    break;
                default:
                    fprintf(stderr, "Unknown option '%s'\n", argv[i]);
                    free(clp);
                    return RET_NULL;
                    break;
                }
            }
                // we don't have a second '-'
            case 'h':
                if (strcmp(&argv[i][1], "help") == 0 || strcmp(&argv[i][1], "h") == 0)
                {
                    clp->options[_OPT_HELP].provided = mtrue; // the program asks for help
                }
                break;
            case 'v':
                if (strcmp(&argv[i][1], "v") == 0 || strcmp(&argv[i][1], "version") == 0)
                {
                    clp->options[_OPT_VER].provided = mtrue; // the program asks for help
                }
                break;
            case 'f':
                // the following option after this must be a destination to the input file
                if (argc < (i + 2))
                {
                    fprintf(stderr, "Expected path to input file after '-f' option, got EOF instead.\n");
                    free(clp);
                    return RET_NULL;
                }
                clp->options[_OPT_FILE].provided = mtrue;
                clp->options[_OPT_FILE]._given_value_str_ = &argv[i + 1];
                i++;
                break;
            case 'l':
                clp->options[_OPT_ENABLE_LOGGER].provided = mtrue;
                break;
            default:
                fprintf(stderr, "Unknown option '%s'\n", argv[i]);
                free(clp);
                return RET_NULL;
            }
            break;
        }
        default:
            fprintf(stderr, "Unknown option '%s'\n", argv[i]);
            free(clp);
            return RET_NULL;
        }
    }
    return clp;
}

void merry_print_help()
{
    fprintf(stdout,
            "Usage; merry -f <path_to_input_file> [OPTIONS]\n"
            "OPTIONS:\n"
            "-h, --h, -help, --help --> Print this help\n"
            "-v, --v, -v, --v       --> Display the current version\n"
            "-f                     --> Provide the path to the input file\n"
            "-l                     --> Enable logging of the VM[Disabled for now. Doesn't work]"
            "\n                           Enabling logging doesn't ensure that a log file will be generated."
            "                           The performance will be affected.\n");
}

void merry_destroy_parser(MerryCLP *clp)
{
    free(clp);
}
