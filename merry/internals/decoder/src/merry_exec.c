#include "../merry_exec.h"

// definitions

_exec_(halt)
{
   // push a halting request to the queue
   merry_requestHdlr_push_request(_REQ_REQHALT, core->core_id, core->cond);
}