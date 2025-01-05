#include <merry.h>
#include <merry_os.h>
#include "merry_temp.h" // we need to rewrite this as well

_MERRY_INTERNAL_ msize_t entry = 0;
_MERRY_INTERNAL_ mbool_t _os_init = mfalse;
_MERRY_INTERNAL_ MerryCLP *cmd_opts = NULL;
_MERRY_INTERNAL_ MerryThread *os_thread = NULL;

// In case this is a child process
mret_t merry_main_parse_options(int argc, char **argv);
void merry_cleanup_and_exit(msize_t ret);
int merry_main();
mret_t merry_get_and_set_id();

// int main()
int main(int argc, char **argv)
{
    // int argc = 3;
    // char *argv[] = {"gdh", "-f", "filetest.mbin"};
    // merry_save_cmd_options(argc, argv); // should this fail then no more debugging the child processes
    inlog("VM intialization started....");
    if (merry_get_and_set_id() == RET_FAILURE)
        return 1;
    if (merry_main_parse_options(argc, argv) == RET_FAILURE)
        merry_cleanup_and_exit(1);
    // This is the main process
    merry_cleanup_and_exit(merry_main());
}

mret_t merry_main_parse_options(int argc, char **argv)
{
    cmd_opts = merry_parse_options(argc, argv, mfalse);
    if (cmd_opts == RET_NULL)
    {
        mreport("Command Line Option parsing error.");
        return RET_FAILURE;
    }
    // if help is to be printed, ignore every other options
    if (cmd_opts->help == mtrue)
    {
        merry_print_help();
        merry_cleanup_and_exit(0);
    }
    // if version is to be printed, do the same as help
    if (cmd_opts->version == mtrue)
    {
        fprintf(stdout, "Merry Virtual Machine: A 64-bit virtual machine\nLatest version-%lu %s\n", _MERRY_VERSION_, _MERRY_VERSION_STATE_);
        merry_cleanup_and_exit(0);
    }
    // see if input file was provided or not
    if (cmd_opts->inp_file == NULL)
    {
        mreport("Expected path to input file, provided none.");
        merry_print_help();
        goto _err;
    }
    log("Received Input File: %s'%s'%s", BOLDWHITE, cmd_opts->inp_file, RESET);
    goto _success;
_err:
    merry_destroy_parser(cmd_opts);
    return RET_FAILURE;
_success:
    return RET_SUCCESS;
}

void merry_cleanup_and_exit(msize_t ret)
{
    merry_destroy_parser(cmd_opts);
    if (_os_init == mtrue)
    {
        merry_thread_destroy(os_thread);
        merry_os_destroy();
    }
    exit(ret);
}

int merry_main()
{
    inlog("Pre-Initialization[VM about to start execution]");
    if (merry_os_init(cmd_opts->inp_file, (cmd_opts->option_count > 0) ? cmd_opts->options : NULL, cmd_opts->option_count, cmd_opts->freeze) == RET_FAILURE)
        return 1;
    _os_init = mtrue;
    if (cmd_opts->dump == mtrue)
        merry_os_produce_dump(cmd_opts->dump_file);
    os_thread = merry_thread_init();
    if (os_thread == NULL) // We can never be sure that this will always work so no debug assertions
    {
        mreport("Manager Thread failed to initialize.");
        return 1;
    }
    inlog("Attempting to start the Manager(OS) Thread");
    if (merry_create_thread(os_thread, &merry_os_start_vm, NULL) == RET_FAILURE)
    {
        mreport("After-initialization failure[The Manager Thread failed to start]");
        return 1;
    }
    inlog("Started execution....");
    inlog("Waiting for the VM to finish...");
    merry_thread_join(os_thread, NULL);
    return merry_os_get_ret();
}

mret_t merry_get_and_set_id()
{
    inlog("Obtaining a new OS ID.");
    FILE *f = fopen(_MERRY_ID_FILE_, "w");
    if (f == RET_NULL)
    {
        mreport("Failed to obtain an OS ID from the ID FILE.");
        return RET_FAILURE;
    }
    // since this is the first process opening it, we don't need lockings and shit
    mbyte_t _init = 0;
    fwrite(&_init, 1, 1, f);
    fclose(f);
    log("Obtained a new OS ID %d.", _init);
    return RET_SUCCESS;
}