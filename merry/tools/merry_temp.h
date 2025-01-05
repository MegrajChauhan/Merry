#ifndef _MERRY_TMP_
#define _MERRY_TMP_

#include "merry_utils.h"
#include "merry_types.h"
#include <stdlib.h>
#include <string.h>
#include "merry_config.h"
#include "merry_console.h"

#ifdef _USE_WIN_
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#endif

mret_t lock_file(FILE *file);

mret_t unlock_file(FILE *file);

#endif