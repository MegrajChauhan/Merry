#ifndef _MERRY_TRAPS_
#define _MERRY_TRAPS_

#define _WIN32_WINNT 0x0500

#include "merry_config.h"
#include "merry_types.h"
#include "merry_ihdlr.h"
#include <windows.h>
#include <stdlib.h>

static PVOID handle = NULL;

mret_t merry_trap_install();

BOOL WINAPI merry_handle_ctrl_event(DWORD ctrlType);

LONG WINAPI merry_handle_exception(EXCEPTION_POINTERS* ExceptionInfo);

#endif