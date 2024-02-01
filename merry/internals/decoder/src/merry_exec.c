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
   core->registers[core->ir.op1] = (mqword_t)(core->registers[core->ir.op1] % core->ir.op2);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(mod_reg)
{
   if (core->registers[core->ir.op2] == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      return;
   }
   // core->registers[core->ir.op1] = (mqword_t)(_mod_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]));
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(iadd_imm)
{
   // The processor will treat op1 and op2 as signed values
   // Since we will get a result that is also signed, we don't have to worry about anything
   core->registers[core->ir.op1] = _add_inst_(core->registers[core->ir.op1], core->ir.op2);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(iadd_reg)
{
   core->registers[core->ir.op1] = _add_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(isub_imm)
{
   core->registers[core->ir.op1] = _sub_inst_(core->registers[core->ir.op1], core->ir.op2);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(isub_reg)
{
   core->registers[core->ir.op1] = _sub_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]);
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
   /// TODO: This logic doesn't work, implement my own imod
   if (core->ir.op2 == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      return; // failure
   }
   // core->registers[core->ir.op1] = _imod_inst_(core->registers[core->ir.op1], core->ir.op2);
   _update_flags_(&core->flag); // the flag register should be updated
}

_exec_(imod_reg)
{
   if (core->registers[core->ir.op2] == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      return;
   }
   // core->registers[core->ir.op1] = _imod_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2]);
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

_exec_(movesx_reg16)
{
   mqword_t lbyte = core->registers[core->ir.op2] & 0xFFFF;
   if (lbyte >> 15 == 1)
      lbyte = _sign_extend16_(lbyte);
   core->registers[core->ir.op1] = lbyte;
}

_exec_(movesx_reg32)
{
   mqword_t lbyte = core->registers[core->ir.op2] & 0xFFFFFF;
   if (lbyte >> 31 == 1)
      lbyte = _sign_extend32_(lbyte);
   core->registers[core->ir.op1] = lbyte;
}

// we don't need zero extended version but we do need the sign extended version of MOV

_exec_(call)
{
   // we need to save the current state of the SP and BP
   // the parameters for the call should be already pushed
   // the parameters can then be accessed using the sva instruction

   /// NOTE: It is to be made sure that the parameters and values of other procedures should not be meddled with
   if (_is_stack_full_(core) || _check_stack_lim_(core, 4))
   {
      // the stack is full and we cannot perform this call
      // the stack must at least have 5 addresses free to be able to perform a call
      merry_requestHdlr_panic(MERRY_STACK_OVERFLOW); // panic
      return;
   }
   core->stack_mem[core->sp] = core->bp; // save the BP
   core->bp = core->sp++;
}

_exec_(ret)
{
   // Restore everything to its older state
   // also check if the stack is empty
   if (_is_stack_empty_(core) || !_stack_has_atleast_(core, 1))
   {
      merry_requestHdlr_panic(MERRY_STACK_UNDERFLOW);
      return;
   }
   core->bp = core->stack_mem[core->bp]; // restore the BP
   // it is the program's job to restore SP to its desired position
}

_exec_(sva)
{
   // op1 is the destination register and op2 is the offset value
   // first check if the offset value is valid
   if (core->bp < core->ir.op2)
   {
      merry_requestHdlr_panic(MERRY_INVALID_VARIABLE_ACCESS);
      return;
   }
   // now we can put the value from the stack
   /// NOTE: We are not popping the values and the original values cannot be changed here
   /// NOTE: Even the value of BP can be taken but in case of svc, the value of BP can be changed which is undesirable
   core->registers[core->ir.op1] = core->stack_mem[core->bp - core->ir.op2];
}

_exec_(svc)
{
   // the same as sva
   if (core->bp < core->ir.op2)
   {
      merry_requestHdlr_panic(MERRY_INVALID_VARIABLE_ACCESS);
      return;
   }
   core->stack_mem[core->bp - core->ir.op2] = core->registers[core->ir.op1];
}

_exec_(push_imm)
{
   // push the provided immediate value onto the stack
   // it is assumed that the value is unsigned and hence zero extended
   if (_is_stack_full_(core))
   {
      merry_requestHdlr_panic(MERRY_STACK_OVERFLOW);
      return; // failure
   }
   core->stack_mem[core->sp++] = core->ir.op1;
}

_exec_(push_reg)
{
   // same as above
   if (_is_stack_full_(core))
   {
      merry_requestHdlr_panic(MERRY_STACK_OVERFLOW);
      return; // failure
   }
   core->stack_mem[core->sp++] = core->registers[core->ir.op1];
}

_exec_(pop)
{
   // the oppsite of above
   if (_is_stack_empty_(core))
   {
      merry_requestHdlr_panic(MERRY_STACK_UNDERFLOW);
      return; // failure
   }
   core->registers[core->ir.op1] = core->stack_mem[core->sp--];
}

_exec_(pusha)
{
   if (!_check_stack_lim_(core, REGR_COUNT))
   {
      // the stack cannot hold all the register values
      merry_requestHdlr_panic(MERRY_STACK_OVERFLOW);
      return;
   }
   for (msize_t i = 0; i < REGR_COUNT; i++)
   {
      // now move one by one
      core->stack_mem[core->sp++] = core->registers[i];
   }
}

_exec_(popa)
{
   if (!_stack_has_atleast_(core, REGR_COUNT))
   {
      // the stack doesn't have enough values
      merry_requestHdlr_panic(MERRY_STACK_UNDERFLOW);
      return;
   }
   for (msize_t i = 15; i >= 0; i--)
   {
      // now move one by one
      core->registers[i] = core->stack_mem[core->sp--];
   }
}

_exec_(and_imm)
{
   // very simple
   core->registers[core->ir.op1] &= core->ir.op2;
}

_exec_(and_reg)
{
   core->registers[core->ir.op1] &= core->registers[core->ir.op2];
}

_exec_(or_imm)
{
   core->registers[core->ir.op1] |= core->ir.op2;
}

_exec_(or_reg)
{
   core->registers[core->ir.op1] |= core->registers[core->ir.op2];
}

_exec_(xor_imm)
{
   core->registers[core->ir.op1] ^= core->ir.op2;
}

_exec_(xor_reg)
{
   core->registers[core->ir.op1] ^= core->registers[core->ir.op2];
}

_exec_(not )
{
   core->registers[core->ir.op1] = ~core->registers[core->ir.op1];
}

_exec_(lshift)
{
   core->registers[core->ir.op1] <<= core->ir.op2;
}

_exec_(rshift)
{
   core->registers[core->ir.op1] >>= core->ir.op2;
}

_exec_(cmp_imm)
{
   _cmp_inst_(core->registers[core->ir.op1], core->ir.op2, &core->flag);
}

_exec_(cmp_reg)
{
   _cmp_inst_(core->registers[core->ir.op1], core->registers[core->ir.op2], &core->flag);
}

_exec_(inc)
{
   core->registers[core->ir.op1] = _inc_inst_(core->registers[core->ir.op1]);
}

_exec_(dec)
{
   core->registers[core->ir.op1] = _dec_inst_(core->registers[core->ir.op1]);
}

_exec_(lea)
{
   // op1 is the destination register
   // op2 is the base
   // Oop3 is the index
   // flag is the scale
   // all these values are kept in a register
   core->registers[core->ir.op1] = core->ir.op2 + core->ir.Oop3 * core->ir.flag;
}
