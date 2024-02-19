/*
 * Definition for functions that perform some request of the Merry VM
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
#ifndef _MERRY_OS_EXEC_
#define _MERRY_OS_EXEC_

#include "merry_os.h"
#include "../../utils/merry_logger.h"

// an function that executes a requests needs to tell the OS if the request was successfully executed

#define _os_exec_(reqname) mret_t merry_os_execute_request_##reqname(Merry *os, MerryOSRequest *request)

// r, r+, w, w+, a, a+
#define _openmode_(bits) (bits == 1) ? "r+" : (bits == 2) ? "w"  \
                                          : (bits == 3)   ? "w+" \
                                          : (bits == 4)   ? "a"  \
                                          : (bits == 5)   ? "a+" \
                                                          : "r"

// handle the halt request
_os_exec_(halt);
_os_exec_(new_core);
_os_exec_(dynl);
_os_exec_(dynul);
_os_exec_(dyncall);
_os_exec_(fopen);
_os_exec_(fclose);
_os_exec_(fread);
_os_exec_(fwrite);
_os_exec_(feof);

#endif
