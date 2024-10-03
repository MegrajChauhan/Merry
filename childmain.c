#include <merry.h>
#include <merry_os.h>
// #include "merry_temp.h"

_MERRY_INTERNAL_ MerryCLP *cmd_opts = NULL;
_MERRY_INTERNAL_ MerryThread *os_thread = NULL;
_MERRY_INTERNAL_ mbool_t _os_init = mfalse;
_MERRY_INTERNAL_ mbyte_t id = 0;

int merry_main();
void merry_cleanup_and_exit(msize_t ret);
mret_t merry_main_parse_options(int argc, char **argv);
mret_t merry_get_and_set_id();

int main(int argc, char **argv)
{
    // we will do things differently with child main
    if (merry_get_and_set_id() == RET_FAILURE)
    {
        inerr_log("CHILD ERROR: Failed to provide ID to the Manager.\n");
        return 1;
    }
    if (merry_main_parse_options(argc, argv) == RET_FAILURE)
        merry_cleanup_and_exit(1);
    merry_cleanup_and_exit(merry_main());
}

mret_t merry_main_parse_options(int argc, char **argv)
{
    cmd_opts = merry_parse_options(argc, argv, mtrue);
    if (cmd_opts == RET_NULL)
    {
        merry_print_help();
        return RET_FAILURE;
    }
    // if help is to be printed, ignore every other options
    if (cmd_opts->_help == mtrue)
    {
        merry_print_help();
        merry_cleanup_and_exit(0);
    }
    // if version is to be printed, do the same as help
    if (cmd_opts->_version == mtrue)
    {
        fprintf(stdout, "Merry Virtual Machine: A 64-bit virtual machine\nLatest version-%lu %s\n", _MERRY_VERSION_, _MERRY_VERSION_STATE_);
        merry_cleanup_and_exit(0);
    }
    // see if input file was provided or not
    if (cmd_opts->_inp_file == NULL)
    {
        fprintf(stderr, "Error: Expected path to input file, provided none\n");
        merry_print_help();
        goto _err;
    }
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
    merry_thread_destroy(os_thread);
    if (_os_init == mtrue)
        merry_os_destroy();
    exit(ret);
}

int merry_main()
{
    MerryReader *r = merry_init_reader(cmd_opts->_inp_file);
    if (r == RET_NULL)
        return 1;
    if (merry_reader_read_file(r) == RET_FAILURE)
    {
        merry_destroy_reader(r);
        return 1;
    }
    _os_init = mtrue;
    // This is the child process and hence we need to modify the EAT
    r->eat.eat_entry_count = 1;
    r->eat.EAT[0] = strtoull(cmd_opts->entry == NULL ? "0" : cmd_opts->entry, NULL, 10); // start from there
    if (merry_os_init_reader_provided(r) == RET_FAILURE)
    {
        inerr_log("Internal Error: Child process failed to start.\n");
        return 1;
    }
    if (cmd_opts->_dump == mtrue)
        merry_os_produce_dump(cmd_opts->_dump_file);
    merry_os_give_id(id);
    os_thread = merry_thread_init();
    if (os_thread == RET_NULL)
    {
        inerr_log("Internal Error: Child process failed to start after initialization.\n");
        return 1;
    }
    if (merry_create_thread(os_thread, &merry_os_start_vm, NULL) == RET_FAILURE)
    {
        inerr_log("Internal Error: Child process failed to start after initialization.\n");
        return 1;
    }
    merry_thread_join(os_thread, NULL);
    return merry_os_get_ret();
}

mret_t merry_get_and_set_id()
{
    FILE *f = fopen(_MERRY_ID_FILE_, "r");
    if (f == RET_NULL)
        return RET_FAILURE;
    if (lock_file(f) == RET_FAILURE)
    {
        fclose(f);
        return RET_FAILURE;
    }
    fread(&id, 1, 1, f);
    mbyte_t _init = id + 1;
    fwrite(&_init, 1, 1, f);
    if (unlock_file(f) == RET_FAILURE)
    {
        fclose(f);
        return RET_FAILURE;
    }
    fclose(f);
    return RET_SUCCESS;
}