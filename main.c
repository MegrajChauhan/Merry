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

_MERRY_INTERNAL_ mbool_t _is_child = mfalse;
_MERRY_INTERNAL_ mbool_t _child_dbg = mfalse;
_MERRY_INTERNAL_ mbool_t _child_freeze = mfalse;
_MERRY_INTERNAL_ msize_t iport = 0;
_MERRY_INTERNAL_ msize_t oport = 0;
_MERRY_INTERNAL_ msize_t entry = 0;
_MERRY_INTERNAL_ mbool_t _os_init = mfalse;

void __get_env_info();
void __set_env_info();

int childmain();

_MERRY_INTERNAL_ MerryCLP *cmd_opts = NULL;
_MERRY_INTERNAL_ MerryThread *os_thread = NULL;

// In case this is a child process

void merry_setup_host(int argc, char **argv);
mret_t merry_main_parse_options(int argc, char **argv);
void merry_cleanup_and_exit(msize_t ret);
int merry_main();

// int main()
int main(int argc, char **argv)
{
    // int argc = 3;
    // char *argv[] = {"gdh", "-f", "subsystest.mbin"};
    merry_setup_host(argc, argv);
    if (merry_main_parse_options(argc, argv) == RET_FAILURE)
        merry_cleanup_and_exit(1);
    if (_is_child == mtrue)
        merry_cleanup_and_exit(childmain());
    // This is the main process
    merry_cleanup_and_exit(merry_main());
}

int childmain()
{
    __set_env_info(); // reset it for child processes
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
    r->eat.EAT[0] = entry; // start from there
    r->de_flag = r->dfe_flag = _child_dbg;
    r->dfw_flag = cmd_opts->freeze = _child_freeze;
    if (merry_os_init_reader_provided(r, iport, oport) == RET_FAILURE)
    {
        inerr_log("Internal Error: Child process failed to start.\n");
        return 1;
    }
    if (cmd_opts->_dump == mtrue)
        merry_os_produce_dump(cmd_opts->_dump_file);
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

void __get_env_info()
{
#ifdef _USE_WIN_
    char _data[16];
    if (GetEnvironmentVariable("_MERRY_CHILD_SURVEY_", _data, sizeof(_data)) < 0)
        return;
    if (strcmp(_data, "no") == 0)
        return; // we have nothing else to do
    // this is indeed a child process
    memset(_data, 0, sizeof(_data));
    _is_child = mtrue;
    GetEnvironmentVariable("_MERRY_CHILD_DEBUG_", _data, sizeof(_data));
    if (strcmp(_data, "no") == 0)
        return;
    memset(_data, 0, sizeof(_data));
    _child_dbg = mtrue;
    GetEnvironmentVariable("_MERRY_CHILD_FREEZE_", _data, sizeof(_data));
    if (strcmp(_data, "no") == 0)
        return;
    memset(_data, 0, sizeof(_data));
    _child_freeze = mtrue;
    GetEnvironmentVariable("_MERRY_CHILD_FREEZE_", _data, sizeof(_data));
    if (strcmp(_data, "no") == 0)
        return;
    memset(_data, 0, sizeof(_data));
    _child_freeze = mtrue;
    GetEnvironmentVariable("_MERRY_IPORT_", _data, sizeof(_data));
    iport = strtoull(_data, NULL, 16); // the value is not invalid
    memset(_data, 0, sizeof(_data));
    GetEnvironmentVariable("_MERRY_OPORT_", _data, sizeof(_data));
    oport = strtoull(_data, NULL, 16); // the value is not invalid
    memset(_data, 0, sizeof(_data));
    GetEnvironmentVariable("_MERRY_ADDR_", _data, sizeof(_data));
    entry = strtoull(_data, NULL, 16); // the value is not invalid
#elif defined(_USE_LINUX_)
    mstr_t _data = getenv("_MERRY_CHILD_SURVEY_");
    if ((_data) == NULL)
        return;
    if (strcmp(_data, "no") == 0)
        return; // we have nothing else to do
    // this is indeed a child process
    _is_child = mtrue;
    _data = getenv("_MERRY_CHILD_DEBUG_");
    if (strcmp(_data, "yes") == 0)
        _child_dbg = mtrue;

    _data = getenv("_MERRY_CHILD_FREEZE_");
    if (strcmp(_data, "yes") == 0)
        _child_freeze = mtrue;
    _child_freeze = mtrue;
    _data = getenv("_MERRY_IPORT_");
    iport = strtoull(_data, NULL, 16); // the value is not invalid
    _data = getenv("_MERRY_OPORT_");
    oport = strtoull(_data, NULL, 16); // the value is not invalid
    _data = getenv("_MERRY_ADDR_");
    entry = strtoull(_data, NULL, 16); // the value is not invalid
#endif
}

void __set_env_info()
{
#ifdef _USE_WIN_
    SetEnvironmentVariable("_MERRY_CHILD_DEBUG_", "no");
    SetEnvironmentVariable("_MERRY_CHILD_FREEZE_", "no");
    SetEnvironmentVariable("_MERRY_IPORT_", "0");
    SetEnvironmentVariable("_MERRY_OPORT_", "0");
    SetEnvironmentVariable("_MERRY_CHILD_SURVEY_", "no");
    SetEnvironmentVariable("_MERRY_ADDR_", "0");
#elif defined(_USE_LINUX_)
    setenv("_MERRY_CHILD_DEBUG_", "no", 1);
    setenv("_MERRY_CHILD_FREEZE_", "no", 1);
    setenv("_MERRY_IPORT_", "0", 1);
    setenv("_MERRY_OPORT_", "0", 1);
    setenv("_MERRY_CHILD_SURVEY_", "no", 1);
    setenv("_MERRY_ADDR_", "0", 1);
#endif
}

void merry_setup_host(int argc, char **argv)
{
    __get_env_info();                   // get any in case this is a child process
    __set_env_info();                   // init env variables
    merry_save_cmd_options(argc, argv); // should this fail then no more debugging the child processes
}

mret_t merry_main_parse_options(int argc, char **argv)
{
    cmd_opts = merry_parse_options(argc, argv);
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
    merry_clean_state();
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