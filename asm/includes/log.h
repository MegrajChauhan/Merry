#ifndef _LOG_
#define _LOG_

#include <stdio.h>
#include "info.h"

#define FATAL "Fatal"
#define WARN "Warning"
#define NOTE "Note"

#define note(k, msg) fprintf(stdout, "%s: %s\n", k, msg)

void log(CompUnit *unit, mstr_t kind, mstr_t msg);

#endif