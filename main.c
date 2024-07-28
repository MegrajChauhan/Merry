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

int main(int argc, char **argv)
{
    // MerryCLP *_parsed_options = merry_parse_options(argc, argv);
    char *x[] = {"./merry", "-f", "temp.bin","--dump-file"};

    MerryCLP *_parsed_options = merry_parse_options(4, x);

    if (_parsed_options == RET_NULL)
    {
        merry_print_help();
        return -1;
    }
    // if help is to be printed, ignore every other options
    if (_parsed_options->_help == mtrue)
    {
        merry_print_help();
        merry_destroy_parser(_parsed_options);
        return 0;
    }
    // if version is to be printed, do the same as help
    if (_parsed_options->_version == mtrue)
    {
        fprintf(stdout, "Merry Virtual Machine: A 64-bit virtual machine\nLatest version-%lu %lu\n", _MERRY_VERSION_, _MERRY_VERSION_STATE_);
        merry_destroy_parser(_parsed_options);
        return 0;
    }
    // see if input file was provided or not
    if (_parsed_options->_inp_file == NULL)
    {
        fprintf(stderr, "Error: Expected path to input file, provided none\n");
        merry_print_help();
        merry_destroy_parser(_parsed_options);
        return -1;
    }
    // Now since we don't have any fancy or complex options to handle, let's get straight to business

    if (merry_os_init(_parsed_options->_inp_file, (_parsed_options->option_count > 0) ? _parsed_options->_options_ : NULL, _parsed_options->option_count, mtrue) == RET_FAILURE)
    {
        // the valid error messages will be automatically printed
        merry_destroy_parser(_parsed_options);
        return -1;
    }
    if (_parsed_options->_dump == mtrue)
        merry_os_produce_dump(_parsed_options->_dump_file);
    // if (merry_os_init("build/isEven.mbin") == RET_FAILURE)
    // {
    //     return -1;
    // }
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
    mqptr_t returnval;
    merry_thread_join(osthread, returnval); // I am an idiot
    merry_thread_destroy(osthread);
    merry_destroy_parser(_parsed_options);
    merry_os_destroy();
    return *returnval;
failure:
    merry_os_destroy();
    return -1;
}