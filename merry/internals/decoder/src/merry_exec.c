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

_exec_(add_imm)
{
   // add immediate value to a register
   core->registers[core->ir.op1] = (mqword_t)(_add_inst_(core->registers[core->ir.op1], core->ir.op2));
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(add_reg)
{
   // add one register to another register
   core->registers[core->ir.op1] = (mqword_t)(_add_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]));
   _update_flags_(&core->flag); // the flag register should be updated
}