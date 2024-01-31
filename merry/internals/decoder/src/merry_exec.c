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

_exec_(sub_imm)
{
   core->registers[core->ir.op1] = (mqword_t)(_sub_inst_(core->registers[core->ir.op1], core->ir.op2));
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(sub_reg)
{
   core->registers[core->ir.op1] = (mqword_t)(_sub_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]));
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(mul_imm)
{
   core->registers[core->ir.op1] = (mqword_t)(_mul_inst_(core->registers[core->ir.op1], core->ir.op2));
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(mul_reg)
{
   core->registers[core->ir.op1] = (mqword_t)(_mul_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]));
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(div_imm)
{
   // We need to add checks here to check if the operands are valid or not
   if (core->ir.op2 == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      return; // failure
   }
   core->registers[core->ir.op1] = (mqword_t)(_div_inst_(core->registers[core->ir.op1], core->ir.op2));
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(div_reg)
{
   if (core->registers[core->ir.op2] == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      return;
   }
   core->registers[core->ir.op1] = (mqword_t)(_div_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]));
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(mod_imm)
{
   // We need to add checks here to check if the operands are valid or not
   if (core->ir.op2 == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      return; // failure
   }
   core->registers[core->ir.op1] = (mqword_t)(_mod_inst_(core->registers[core->ir.op1], core->ir.op2));
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(mod_reg)
{
   if (core->registers[core->ir.op2] == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      return;
   }
   core->registers[core->ir.op1] = (mqword_t)(_mod_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]));
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(iadd_imm)
{
   // The processor will treat op1 and op2 as signed values
   // Since we will get a result that is also signed, we don't have to worry about anything
   core->registers[core->ir.op1] = _iadd_inst_(core->registers[core->ir.op1], core->ir.op2);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(iadd_reg)
{
   core->registers[core->ir.op1] = _iadd_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(isub_imm)
{
   core->registers[core->ir.op1] = _isub_inst_(core->registers[core->ir.op1], core->ir.op2);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(isub_reg)
{
   core->registers[core->ir.op1] = _isub_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(imul_imm)
{
   core->registers[core->ir.op1] = _imul_inst_(core->registers[core->ir.op1], core->ir.op2);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(imul_reg)
{
   core->registers[core->ir.op1] = _imul_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(idiv_imm)
{
   if (core->ir.op2 == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      return; // failure
   }
   core->registers[core->ir.op1] = _idiv_inst_(core->registers[core->ir.op1], core->ir.op2);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(idiv_reg)
{
   if (core->registers[core->ir.op2] == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      return;
   }
   core->registers[core->ir.op1] = _idiv_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(imod_imm)
{
   if (core->ir.op2 == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      return; // failure
   }
   core->registers[core->ir.op1] = _imod_inst_(core->registers[core->ir.op1], core->ir.op2);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(imod_reg)
{
   if (core->registers[core->ir.op2] == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      return;
   }
   core->registers[core->ir.op1] = _imod_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(move_imm)
{
   core->registers[core->ir.op1] = core->ir.op2; // this is all
}

_exec_(move_reg)
{
   core->registers[core->ir.op1] = core->registers[core->ir.op2]; // this is all
}

_exec_(move_reg8)
{
   core->registers[core->ir.op1] = core->registers[core->ir.op2] & 0xFF; // this is all
}

_exec_(move_reg16)
{
   core->registers[core->ir.op1] = core->registers[core->ir.op2] & 0xFFFF; // this is all
}

_exec_(move_reg32)
{
   core->registers[core->ir.op1] = core->registers[core->ir.op2] & 0xFFFFFF; // this is all
}

_exec_(movesx_imm8)
{
   mqword_t lbyte = core->ir.op2; // this is only 1 byte long
   if (lbyte >> 7 == 1)
      lbyte = _sign_extend8_(lbyte);
   core->registers[core->ir.op1] = lbyte; // this is all
}

_exec_(movesx_imm16)
{
   mqword_t lbyte = core->ir.op2; // this is 2 byte long
   if (lbyte >> 15 == 1)
      lbyte = _sign_extend16_(lbyte);
   core->registers[core->ir.op1] = lbyte;
}

_exec_(movesx_imm32)
{
   mqword_t lbyte = core->ir.op2; // this is 2 byte long
   if (lbyte >> 31 == 1)
      lbyte = _sign_extend32_(lbyte);
   core->registers[core->ir.op1] = lbyte;
}

_exec_(movesx_reg8)
{
   mqword_t lbyte = core->registers[core->ir.op2] & 0xFF;
   if (lbyte >> 7 == 1)
      lbyte = _sign_extend8_(lbyte);
   core->registers[core->ir.op1] = lbyte;
}

_exec_(move_reg16)
{
   mqword_t lbyte = core->registers[core->ir.op2] & 0xFFFF;
   if (lbyte >> 15 == 1)
      lbyte = _sign_extend16_(lbyte);
   core->registers[core->ir.op1] = lbyte;
}

_exec_(move_reg32)
{
   mqword_t lbyte = core->registers[core->ir.op2] & 0xFFFFFF;
   if (lbyte >> 31 == 1)
      lbyte = _sign_extend32_(lbyte);
   core->registers[core->ir.op1] = lbyte;
}

// we don't need zero extended version but we do need the sign extended version of MOV
