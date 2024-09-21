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
#ifndef _MERRY_TMP_
#define _MERRY_TMP_

/**
 * This acts as a placeholder for common and default values.
 * We have yet to make the VM configurable.
 * This is where we store one-time temporary data
 */

#include "merry_utils.h"
#include "merry_types.h"
#include <stdlib.h>
#include <string.h>

_MERRY_INTERNAL_ mstr_t *_cmd_opts = NULL;
_MERRY_INTERNAL_ msize_t _cmd_opt_len = 0;

// The thing is, if the following fails, any new process will not be created
void merry_save_cmd_options(msize_t argc, mstr_t *argv);

void merry_get_cmd_options(msize_t *argc, mstr_t **argv);

void merry_clean_state();

#endif