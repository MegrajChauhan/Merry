#include "merry/internals/merry.h"

int main(int argc, char **argv)
{
    MerryCLP *_parsed_options = merry_parse_options(argc, argv);
    // char *x[3] = {"./merry", "-f", "help"};

    // MerryCLP *_parsed_options = merry_parse_options(3, x);

    if (_parsed_options == RET_NULL)
    {
        merry_print_help();
        return -1;
    }
    // if help is to be printed, ignore every other options
    if (_parsed_options->options[_OPT_HELP].provided == mtrue)
    {
        merry_print_help();
        merry_destroy_parser(_parsed_options);
        return 0;
    }
    // if version is to be printed, do the same as help
    if (_parsed_options->options[_OPT_VER].provided == mtrue)
    {
        fprintf(stdout, "merry version-%s %s\n", _MERRY_VERSION_, _MERRY_VERSION_STATE_);
        merry_destroy_parser(_parsed_options);
        return 0;
    }
    // see if input file was provided or not
    if (_parsed_options->options[_OPT_FILE].provided == mfalse)
    {
        fprintf(stderr, "Error: Expected path to input file, provided none\n");
        merry_print_help();
        merry_destroy_parser(_parsed_options);
        return -1;
    }
    // Now since we don't have any fancy or complex options to handle, let's get straight to business
    merry_logger_init(_parsed_options->options[_OPT_ENABLE_LOGGER].provided == mtrue ? mtrue : mfalse); // we won't enable logging yet, this is just an option rn
    if (merry_os_init(*_parsed_options->options[_OPT_FILE]._given_value_str_) == RET_FAILURE)
    {
        // the valid error messages will be automatically printed
        merry_destroy_parser(_parsed_options);
        return -1;
    }
    // if (merry_os_init("inpFile.mbin") == RET_FAILURE)
    // {
    //     return -1;
    // }
    merry_destroy_parser(_parsed_options);
    MerryThread *osthread = merry_thread_init();
    if (osthread == NULL)
    {
        fprintf(stderr, "Failed to intialize VM.\n");
        goto failure;
    }
    if (merry_create_thread(osthread, &merry_os_start_vm, NULL) == RET_FAILURE)
    {
        fprintf(stderr, "Failed to start VM.\n");
        merry_thread_destroy(osthread);
        goto failure;
    }
    msize_t returnval = 0;
    merry_logger_close();
    merry_thread_join(osthread, &returnval); // I am an idiot
    merry_thread_destroy(osthread);
    merry_os_destroy();
    // printf("Time taken to run: %lfs.\n", (double)(end - start) / CLOCKS_PER_SEC);
    return returnval;
failure:
    merry_logger_close();
    merry_os_destroy();
    return -1;
}