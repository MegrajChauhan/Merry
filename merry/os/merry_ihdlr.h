#ifndef _MERRY_IHDLR_
#define _MERRY_IHDLR_

#include "merry_config.h"
#include "merry_types.h"
#include "merry_errno.h"
#include "merry_request_hdlr.h"

_MERRY_DEFINE_FUNC_PTR_(void, mihdlr_t, int _s);

#define merry_ihdlr(name) void merry_handle_##name (int _sig)

merry_ihdlr(termination);
merry_ihdlr(interrupt);
merry_ihdlr(segv);

#endif