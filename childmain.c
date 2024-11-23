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
    inlog("VM intialization started[CHILD PROCESS]....");
    if (merry_get_and_set_id() == RET_FAILURE)
        return 1;
    if (merry_main_parse_options(argc, argv) == RET_FAILURE)
        merry_cleanup_and_exit(1);
    merry_cleanup_and_exit(merry_main());
}

mret_t merry_main_parse_options(int argc, char **argv)
{
    // In the child process, the "help" and "version" options are useless but anyway.
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
        mreport("Error: Expected path to input file, provided none.");
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
    merry_thread_destroy(os_thread);
    if (_os_init == mtrue)
        merry_os_destroy();
    exit(ret);
}

int merry_main()
{
    log("Pre-Initialization[VM(Child Process[%d]) about to start execution]", id);
    MerryReader *r = merry_init_reader(cmd_opts->inp_file);
    if (r == RET_NULL)
        return 1;
    log("VM Child Process[%d] attempting to read the input file.", id);
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
        log("New VM child process[ID: %d] failed to start.", id);
        return 1;
    }
    if (cmd_opts->dump == mtrue)
        merry_os_produce_dump(cmd_opts->dump_file);
    merry_os_give_id(id);
    os_thread = merry_thread_init();
    if (os_thread == RET_NULL)
    {
        log("New VM child process[ID: %d] failed to start[Manager Thread failed to initialize].", id);
        return 1;
    }
    log("Attempting to start the Manager(OS ID: %d) Thread", id);
    if (merry_create_thread(os_thread, &merry_os_start_vm, NULL) == RET_FAILURE)
    {
        log("New VM child process[ID: %d] failed to start[Manager Thread failed to start].", id);
        return 1;
    }
    log("Started execution[ID: %d]....", id);
    log("Waiting for the VM[ID: %d] to finish...", id);
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
    log("Obtained a new OS ID %d.", _init);
    return RET_SUCCESS;
}