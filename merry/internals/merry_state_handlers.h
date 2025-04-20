#ifndef _MERRY_STATE_HANDLER_
#define _MERRY_STATE_HANDLER_

#include "merry_state.h"
#include "merry_utils.h"
#include "merry_helpers.h"

void merry_MAKE_SENSE_OF_STATE(MerryState *state);

void merry_HANDLE_INTERNAL_SYS_ERROR(msyserr_t err, MerryPtrToQword arg);

void merry_HANDLE_PROGRAM_ERROR(mprogerr_t err, MerryPtrToQword arg);

void merry_HANDLE_CONTEXT(mcont_t cont, MerryPtrToQword arg);

#endif
