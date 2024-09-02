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