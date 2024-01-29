#include "../merry_exec.h"
#include "../../merry_core.h"

// definitions

_exec_(halt)
{
   // push a halting request to the queue
   _llog_(_CORE_, "EXEC", "Core execuitng halt; core ID %lu", core->core_id);
   merry_requestHdlr_push_request(_REQ_REQHALT, core->core_id, core->cond);
   _llog_(_CORE_, "EXEC_DONE", "Halt execution done; core ID %lu", core->core_id);
}