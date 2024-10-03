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
#include <merry.h>
#include <merry_os.h>
#include "merry_temp.h"

_MERRY_INTERNAL_ msize_t entry = 0;
_MERRY_INTERNAL_ mbool_t _os_init = mfalse;
_MERRY_INTERNAL_ MerryCLP *cmd_opts = NULL;
_MERRY_INTERNAL_ MerryThread *os_thread = NULL;

// In case this is a child process
mret_t merry_main_parse_options(int argc, char **argv);
void merry_cleanup_and_exit(msize_t ret);
int merry_main();
mret_t merry_get_and_set_id();

int main()
// int main(int argc, char **argv)
{
    int argc = 3;
    char *argv[] = {"gdh", "-f", "bin/filetest.mbin"};
    // merry_save_cmd_options(argc, argv); // should this fail then no more debugging the child processes
    if (merry_get_and_set_id() == RET_FAILURE)
    {
        inerr_log("ERROR: Failed to provide ID to the Manager.\n");
        return 1;
    }
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
    if (merry_os_init(cmd_opts->_inp_file, (cmd_opts->option_count > 0) ? cmd_opts->_options_ : NULL, cmd_opts->option_count, cmd_opts->freeze) == RET_FAILURE)
        return 1;
    _os_init = mtrue;
    if (cmd_opts->_dump == mtrue)
        merry_os_produce_dump(cmd_opts->_dump_file);
    os_thread = merry_thread_init();
    if (os_thread == NULL)
    {
        inerr_log("Internal Error: Failed to initialize VM.\n");
        return 1;
    }
    if (merry_create_thread(os_thread, &merry_os_start_vm, NULL) == RET_FAILURE)
    {
        inerr_log("Internal Error: The VM didn't start after initialization.\n");
        return 1;
    }
    merry_thread_join(os_thread, NULL);
    return merry_os_get_ret();
}

mret_t merry_get_and_set_id()
{
    FILE *f = fopen(_MERRY_ID_FILE_, "w");
    if (f == RET_NULL)
        return RET_FAILURE;
    // since this is the first process opening it, we don't need lockings and shit
    mbyte_t _init = 0;
    fwrite(&_init, 1, 1, f);
    fclose(f);
    return RET_SUCCESS;
}