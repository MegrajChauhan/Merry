#if defined(_WIN64)
#include "internals\merry_exec.h"
#include "internals\merry_core.h"
#include "internals\merry_os.h"
#else
#include "internals/merry_exec.h"
#include "internals/merry_core.h"
#include "internals/merry_os.h"
#endif

// definitions
// _MERRY_ALWAYS_INLINE_ _exec_(halt)
// {
//    // push a halting request to the queue
//    _llog_(_CORE_, "EXEC", "Core execuitng halt; core ID %lu", core->core_id);
//    merry_requestHdlr_push_request(_REQ_REQHALT, core->core_id, core->cond);
//    core->stop_running = mtrue;
//    _llog_(_CORE_, "EXEC_DONE", "Halt execution done; core ID %lu", core->core_id);
// }

_MERRY_INTERNAL_ void merry_cmp_floats64(MerryCore *core, double val1, double val2)
{
   if (val1 == val2)
      core->flag.zero = 1; // equal
   else
      core->flag.zero = 0;
   if (val1 > val2)
      core->greater = 1;
   else
      core->greater = 0;
   // only these two flags are affected
}

_MERRY_INTERNAL_ void merry_cmp_floats32(MerryCore *core, float val1, float val2)
{
   if (val1 == val2)
      core->flag.zero = 1; // equal
   else
      core->flag.zero = 0;
   if (val1 > val2)
      core->greater = 1;
   else
      core->greater = 0;
   // only these two flags are affected
}

_MERRY_ALWAYS_INLINE_ _exec_(add_imm){
    // add immediate value to a register
    _ArithMeticImmFrame_(+)}

_MERRY_ALWAYS_INLINE_ _exec_(add_reg){
    // add one register to another register
    _ArithMeticRegFrame_(+)}

_MERRY_ALWAYS_INLINE_ _exec_(sub_imm)
{
   _ArithMeticImmFrame_(-) if (core->flag.negative == 0)
           core->greater == 1;
}

_MERRY_ALWAYS_INLINE_ _exec_(sub_reg)
{
   _ArithMeticRegFrame_(-) if (core->flag.negative == 0)
           core->greater == 1;
}

_MERRY_ALWAYS_INLINE_ _exec_(mul_imm){
    _ArithMeticImmFrame_(*)}

_MERRY_ALWAYS_INLINE_ _exec_(mul_reg){
    _ArithMeticRegFrame_(*)}

_MERRY_ALWAYS_INLINE_ _exec_(div_imm)
{
   // We need to add checks here to check if the operands are valid or not
   register mqword_t current = core->current_inst;
   register mqword_t reg = (current >> 48) & 15;
   register mqword_t imm = current & 0xFFFFFFFF;
   if (imm == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return; // failure
   }
   core->registers[reg] = core->registers[reg] / imm;
   _update_flags_(&core->flag);
}

_MERRY_ALWAYS_INLINE_ _exec_(div_reg)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg = (current >> 4) & 15;
   register mqword_t reg2 = (current) & 15;
   if (core->registers[reg2] == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return;
   }
   core->registers[reg] = core->registers[reg] / core->registers[reg2];
   _update_flags_(&core->flag);
}

_MERRY_ALWAYS_INLINE_ _exec_(mod_imm)
{
   // We need to add checks here to check if the operands are valid or not
   register mqword_t current = core->current_inst;
   register mqword_t reg = (current >> 48) & 15;
   register mqword_t imm = current & 0xFFFFFFFF;
   if (imm == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return; // failure
   }
   core->registers[reg] = core->registers[reg] % imm;
   _update_flags_(&core->flag);
}

_MERRY_ALWAYS_INLINE_ _exec_(mod_reg)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg = (current >> 52) & 15;
   register mqword_t reg2 = (current >> 48) & 15;
   if (core->registers[reg2] == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return;
   }
   core->registers[reg] = core->registers[reg] % core->registers[reg2];
   _update_flags_(&core->flag);
}

_MERRY_ALWAYS_INLINE_ _lexec_(add_mem, mem_read func)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg = (current >> 52) & 15;
   register mqword_t addr = (current & 0xFFFFFFFFFFFF) & 15;
   mqword_t temp = 0;
   if (func(core->data_mem, addr, &temp) == RET_FAILURE)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return;
   }
   core->registers[(current >> 48) & 15] += temp;
   _update_flags_(&core->flag);
}

_MERRY_ALWAYS_INLINE_ _lexec_(sub_mem, mem_read func)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg = (current >> 52) & 15;
   register mqword_t addr = (current & 0xFFFFFFFFFFFF) & 15;
   mqword_t temp = 0;
   if (func(core->data_mem, addr, &temp) == RET_FAILURE)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return;
   }
   core->registers[(current >> 48) & 15] -= temp;
   _update_flags_(&core->flag);
}

_MERRY_ALWAYS_INLINE_ _lexec_(mul_mem, mem_read func)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg = (current >> 52) & 15;
   register mqword_t addr = (current & 0xFFFFFFFFFFFF) & 15;
   mqword_t temp = 0;
   if (func(core->data_mem, addr, &temp) == RET_FAILURE)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return;
   }
   core->registers[(current >> 48) & 15] *= temp;
   _update_flags_(&core->flag);
}

_MERRY_ALWAYS_INLINE_ _lexec_(div_mem, mem_read func)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg = (current >> 52) & 15;
   register mqword_t addr = (current & 0xFFFFFFFFFFFF) & 15;
   mqword_t temp = 0;
   if (func(core->data_mem, addr, &temp) == RET_FAILURE)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return;
   }
   if (temp == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return;
   }
   core->registers[(current >> 48) & 15] /= temp;
   _update_flags_(&core->flag);
}

_MERRY_ALWAYS_INLINE_ _lexec_(mod_mem, mem_read func)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg = (current >> 52) & 15;
   register mqword_t addr = (current & 0xFFFFFFFFFFFF) & 15;
   mqword_t temp = 0;
   if (func(core->data_mem, addr, &temp) == RET_FAILURE)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return;
   }
   if (temp == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return;
   }
   core->registers[(current >> 48) & 15] %= temp;
   _update_flags_(&core->flag);
}

_MERRY_ALWAYS_INLINE_ _exec_(iadd_imm){
    // The processor will treat op1 and op2 as signed values
    // Since we will get a result that is also signed, we don't have to worry about anything
    _SArithMeticImmFrame_(+)}

_MERRY_ALWAYS_INLINE_ _exec_(iadd_reg){
    _SArithMeticRegFrame_(+)}

_MERRY_ALWAYS_INLINE_ _exec_(isub_imm)
{
   _SArithMeticImmFrame_(-) if (core->flag.negative == 0)
           core->greater == 1;
}

_MERRY_ALWAYS_INLINE_ _exec_(isub_reg)
{
   _SArithMeticRegFrame_(-) if (core->flag.negative == 0)
           core->greater == 1;
}

_MERRY_ALWAYS_INLINE_ _exec_(imul_imm){
    _SArithMeticImmFrame_(*)}

_MERRY_ALWAYS_INLINE_ _exec_(imul_reg){
    _SArithMeticRegFrame_(*)}

_MERRY_ALWAYS_INLINE_ _exec_(idiv_imm)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg = (current >> 48) & 15;
   register mqword_t imm = current & _sign_extend32_(0xFFFFFFFF);
   if (imm == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return; // failure
   }
   core->registers[reg] = core->registers[reg] / imm;
   _update_flags_(&core->flag);
}

_MERRY_ALWAYS_INLINE_ _exec_(idiv_reg)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg = (current >> 4) & 15;
   register mqword_t reg2 = (current) & 15;
   if (core->registers[reg2] == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return;
   }
   core->registers[reg] = core->registers[reg] / core->registers[reg2];
   _update_flags_(&core->flag);
}

_MERRY_ALWAYS_INLINE_ _exec_(imod_imm)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg = (current >> 48) & 15;
   register mqword_t imm = current & _sign_extend32_(0xFFFFFFFF);
   if (imm == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return; // failure
   }
   core->registers[reg] = core->registers[reg] % imm;
   _update_flags_(&core->flag);
}

_MERRY_ALWAYS_INLINE_ _exec_(imod_reg)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg = (current >> 4) & 15;
   register mqword_t reg2 = (current) & 15;
   if (core->registers[reg2] == 0)
   {
      merry_requestHdlr_panic(MERRY_DIV_BY_ZERO);
      core->stop_running = mtrue;
      return;
   }
   core->registers[reg] = core->registers[reg] % core->registers[reg2];
   _update_flags_(&core->flag);
}

_MERRY_ALWAYS_INLINE_ _exec_(fadd)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg1 = (current >> 4) & 15;
   register mqword_t reg2 = (current) & 15;
   core->registers[reg1] = ((double)core->registers[reg1] + (double)core->registers[reg2]);
   // we have no flags for floating point arithmetic
   // so we gotta do with the flags register we have
   merry_cmp_floats64(core, core->registers[reg1], core->registers[reg2]);
}

_MERRY_ALWAYS_INLINE_ _exec_(fsub)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg1 = (current >> 4) & 15;
   register mqword_t reg2 = (current) & 15;
   core->registers[reg1] = ((double)core->registers[reg1] - (double)core->registers[reg2]);
   merry_cmp_floats64(core, core->registers[reg1], core->registers[reg2]);
}

_MERRY_ALWAYS_INLINE_ _exec_(fmul)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg1 = (current >> 4) & 15;
   register mqword_t reg2 = (current) & 15;
   core->registers[reg1] = ((double)core->registers[reg1] * (double)core->registers[reg2]);
   merry_cmp_floats64(core, core->registers[reg1], core->registers[reg2]);
}

_MERRY_ALWAYS_INLINE_ _exec_(fdiv)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg1 = (current >> 4) & 15;
   register mqword_t reg2 = (current) & 15;
   core->registers[reg1] = ((double)core->registers[reg1] / (double)core->registers[reg2]);
   merry_cmp_floats64(core, core->registers[reg1], core->registers[reg2]);
}

_MERRY_ALWAYS_INLINE_ _exec_(fadd32)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg1 = (current >> 4) & 15;
   register mqword_t reg2 = (current) & 15;
   core->registers[reg1] = ((float)core->registers[reg1] + (float)core->registers[reg2]);
   merry_cmp_floats32(core, core->registers[reg1], core->registers[reg2]);
}

_MERRY_ALWAYS_INLINE_ _exec_(fsub32)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg1 = (current >> 4) & 15;
   register mqword_t reg2 = (current) & 15;
   core->registers[reg1] = ((float)core->registers[reg1] - (float)core->registers[reg2]);
   merry_cmp_floats32(core, core->registers[reg1], core->registers[reg2]);
}

_MERRY_ALWAYS_INLINE_ _exec_(fmul32)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg1 = (current >> 4) & 15;
   register mqword_t reg2 = (current) & 15;
   core->registers[reg1] = ((float)core->registers[reg1] * (float)core->registers[reg2]);
   merry_cmp_floats32(core, core->registers[reg1], core->registers[reg2]);
}

_MERRY_ALWAYS_INLINE_ _exec_(fdiv32)
{
   register mqword_t current = core->current_inst;
   register mqword_t reg1 = (current >> 4) & 15;
   register mqword_t reg2 = (current) & 15;
   // divide by zero should result in infinity
   core->registers[reg1] = ((float)core->registers[reg1] / (float)core->registers[reg2]);
   merry_cmp_floats32(core, core->registers[reg1], core->registers[reg2]);
}

_MERRY_ALWAYS_INLINE_ _exec_(movesx_imm8)
{
   register mqword_t current = core->current_inst;
   register mqword_t lbyte = current & 0xFF; // this is only 1 byte long
   if (lbyte >> 7 == 1)
      lbyte = _sign_extend8_(lbyte);
   core->registers[(current >> 48) & 15] = lbyte;
}

_MERRY_ALWAYS_INLINE_ _exec_(movesx_imm16)
{
   register mqword_t current = core->current_inst;
   register mqword_t lbyte = current & 0xFFFF;
   if (lbyte >> 15 == 1)
      lbyte = _sign_extend16_(lbyte);
   core->registers[(current >> 48) & 15] = lbyte;
}

_MERRY_ALWAYS_INLINE_ _exec_(movesx_imm32)
{
   register mqword_t current = core->current_inst;
   register mqword_t lbyte = current & 0xFFFFFF;
   if (lbyte >> 31 == 1)
      lbyte = _sign_extend32_(lbyte);
   core->registers[(current >> 48) & 15] = lbyte;
}

_MERRY_ALWAYS_INLINE_ _exec_(movesx_reg8)
{
   register mqword_t current = core->current_inst;
   register mqword_t lbyte = core->registers[(current) & 15] & 0xFF;
   if (lbyte >> 7 == 1)
      lbyte = _sign_extend8_(lbyte);
   core->registers[(current >> 4) & 15] = lbyte;
}

_MERRY_ALWAYS_INLINE_ _exec_(movesx_reg16)
{
   register mqword_t current = core->current_inst;
   register mqword_t lbyte = core->registers[(current) & 15] & 0xFFFF;
   if (lbyte >> 15 == 1)
      lbyte = _sign_extend16_(lbyte);
   core->registers[(current >> 4) & 15] = lbyte;
}

_MERRY_ALWAYS_INLINE_ _exec_(movesx_reg32)
{
   register mqword_t current = core->current_inst;
   register mqword_t lbyte = core->registers[(current) & 15] & 0xFFFFFF;
   if (lbyte >> 31 == 1)
      lbyte = _sign_extend32_(lbyte);
   core->registers[(current >> 4) & 15] = lbyte;
}

// we don't need zero extended version but we do need the sign extended version of MOV

_MERRY_ALWAYS_INLINE_ _exec_(call)
{
   // we need to save the current state of the SP and BP
   // the parameters for the call should be already pushed
   // the parameters can then be accessed using the sva instruction

   /// NOTE: It is to be made sure that the parameters and values of other procedures should not be meddled with
   if (_is_stack_full_(core) || !_check_stack_lim_(core, 4))
   {
      // the stack is full and we cannot perform this call
      // the stack must at least have 5 addresses free to be able to perform a call
      merry_requestHdlr_panic(MERRY_STACK_OVERFLOW); // panic
      core->stop_running = mtrue;
      return;
   }
   core->stack_mem[core->sp] = core->bp; // save the BP
   core->bp = core->sp++;
}

_MERRY_ALWAYS_INLINE_ _exec_(ret)
{
   // Restore everything to its older state
   // also check if the stack is empty
   if (_is_stack_empty_(core) || !_stack_has_atleast_(core, 1))
   {
      merry_requestHdlr_panic(MERRY_STACK_UNDERFLOW);
      core->stop_running = mtrue;
      return;
   }
   core->bp = core->stack_mem[core->bp]; // restore the BP
   // it is the program's job to restore SP to its desired position
}

_MERRY_ALWAYS_INLINE_ _exec_(sva)
{
   // op1 is the destination register and op2 is the offset value
   // first check if the offset value is valid
   register mqword_t current = core->current_inst;
   register mqword_t off = current & 0xFFFF;
   if (core->bp < off)
   {
      merry_requestHdlr_panic(MERRY_INVALID_VARIABLE_ACCESS);
      core->stop_running = mtrue;
      return;
   }
   // now we can put the value from the stack
   /// NOTE: We are not popping the values and the original values cannot be changed here
   /// NOTE: Even the value of BP can be taken but in case of svc, the value of BP can be changed which is undesirable
   core->registers[(current >> 48) & 15] = core->stack_mem[core->bp - off];
}

_MERRY_ALWAYS_INLINE_ _exec_(svc)
{
   // the same as sva
   register mqword_t current = core->current_inst;
   register mqword_t off = current & 0xFFFF;
   if (core->bp < off)
   {
      merry_requestHdlr_panic(MERRY_INVALID_VARIABLE_ACCESS);
      core->stop_running = mtrue;
      return;
   }
   core->stack_mem[core->bp - off] = core->registers[(current >> 48) & 15];
}

_MERRY_ALWAYS_INLINE_ _exec_(push_imm)
{
   // push the provided immediate value onto the stack
   // it is assumed that the value is unsigned and hence zero extended
   if (_is_stack_full_(core))
   {
      merry_requestHdlr_panic(MERRY_STACK_OVERFLOW);
      core->stop_running = mtrue;
      return; // failure
   }
   core->stack_mem[core->sp++] = core->current_inst & 0xFFFFFFFFFFFF;
}

_MERRY_ALWAYS_INLINE_ _exec_(push_reg)
{
   // same as above
   if (_is_stack_full_(core))
   {
      merry_requestHdlr_panic(MERRY_STACK_OVERFLOW);
      core->stop_running = mtrue;
      return; // failure
   }
   core->stack_mem[core->sp++] = core->registers[core->current_inst & 15];
}

_MERRY_ALWAYS_INLINE_ _exec_(pop)
{
   // the oppsite of above
   if (_is_stack_empty_(core))
   {
      merry_requestHdlr_panic(MERRY_STACK_UNDERFLOW);
      core->stop_running = mtrue;
      return; // failure
   }
   core->registers[core->current_inst & 15] = core->stack_mem[core->sp--];
}

_MERRY_ALWAYS_INLINE_ _exec_(pusha)
{
   if (!_check_stack_lim_(core, REGR_COUNT))
   {
      // the stack cannot hold all the register values
      merry_requestHdlr_panic(MERRY_STACK_OVERFLOW);
      core->stop_running = mtrue;
      return;
   }
   for (msize_t i = 0; i < REGR_COUNT; i++)
   {
      // now move one by one
      core->stack_mem[core->sp++] = core->registers[i];
   }
}

_MERRY_ALWAYS_INLINE_ _exec_(popa)
{
   if (!_stack_has_atleast_(core, REGR_COUNT))
   {
      // the stack doesn't have enough values
      merry_requestHdlr_panic(MERRY_STACK_UNDERFLOW);
      core->stop_running = mtrue;
      return;
   }
   for (msize_t i = 15; i != 0; i--)
   {
      // now move one by one
      core->registers[i] = core->stack_mem[core->sp--];
   }
}

_MERRY_ALWAYS_INLINE_ _lexec_(load, mqword_t address)
{
   // read from the given address
   if (merry_dmemory_read_qword(core->data_mem, address, &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be loaded
}

_MERRY_ALWAYS_INLINE_ _lexec_(store, mqword_t address)
{
   // store to the given address from the given register
   if (merry_dmemory_write_qword(core->data_mem, address, core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be stored
}

_MERRY_ALWAYS_INLINE_ _lexec_(loadw, mqword_t address)
{
   // read from the given address
   if (merry_dmemory_read_word(core->data_mem, address, &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be loaded
}

_MERRY_ALWAYS_INLINE_ _lexec_(storew, mqword_t address)
{
   // store to the given address from the given register
   if (merry_dmemory_write_word(core->data_mem, address, core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be stored
}

_MERRY_ALWAYS_INLINE_ _lexec_(loadd, mqword_t address)
{
   // read from the given address
   if (merry_dmemory_read_dword(core->data_mem, address, &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be loaded
}

_MERRY_ALWAYS_INLINE_ _lexec_(stored, mqword_t address)
{
   // store to the given address from the given register
   if (merry_dmemory_write_dword(core->data_mem, address, core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be stored
}

_MERRY_ALWAYS_INLINE_ _lexec_(loadb, mqword_t address)
{
   // read from the given address
   if (merry_dmemory_read_byte(core->data_mem, address, &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be loaded
}

_MERRY_ALWAYS_INLINE_ _lexec_(storeb, mqword_t address)
{
   // store to the given address from the given register
   if (merry_dmemory_write_byte(core->data_mem, address, core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be stored
}

_MERRY_ALWAYS_INLINE_ _lexec_(load_reg, mqword_t address)
{
   // read from the given address
   if (merry_dmemory_read_qword(core->data_mem, address, &core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be loaded
}

_MERRY_ALWAYS_INLINE_ _lexec_(store_reg, mqword_t address)
{
   // store to the given address from the given register
   if (merry_dmemory_write_qword(core->data_mem, address, core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be stored
}

_MERRY_ALWAYS_INLINE_ _lexec_(loadw_reg, mqword_t address)
{
   // read from the given address
   if (merry_dmemory_read_word(core->data_mem, address, &core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be loaded
}

_MERRY_ALWAYS_INLINE_ _lexec_(storew_reg, mqword_t address)
{
   // store to the given address from the given register
   if (merry_dmemory_write_word(core->data_mem, address, core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be stored
}

_MERRY_ALWAYS_INLINE_ _lexec_(loadd_reg, mqword_t address)
{
   // read from the given address
   if (merry_dmemory_read_dword(core->data_mem, address, &core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be loaded
}

_MERRY_ALWAYS_INLINE_ _lexec_(stored_reg, mqword_t address)
{
   // store to the given address from the given register
   if (merry_dmemory_write_dword(core->data_mem, address, core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be stored
}

_MERRY_ALWAYS_INLINE_ _lexec_(loadb_reg, mqword_t address)
{
   // read from the given address
   if (merry_dmemory_read_byte(core->data_mem, address, &core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be loaded
}

_MERRY_ALWAYS_INLINE_ _lexec_(storeb_reg, mqword_t address)
{
   // store to the given address from the given register
   if (merry_dmemory_write_byte(core->data_mem, address, core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be stored
}

_MERRY_ALWAYS_INLINE_ _exec_(excg8)
{
   // just exchange the bytes and don't overwrite anything
   register mqword_t curr = core->current_inst;
   register mqword_t reg1 = core->registers[(curr >> 4) & 15];
   register mqword_t reg2 = core->registers[curr & 15];
   core->registers[(curr >> 4) & 15] &= (0xFFFFFFFFFFFFFF00 | (reg2 & 0xFF));
   core->registers[curr & 15] &= (0xFFFFFFFFFFFFFF00 | (reg1 & 0xFF));
}

_MERRY_ALWAYS_INLINE_ _exec_(excg16)
{
   // just exchange the bytes and don't overwrite anything
   register mqword_t curr = core->current_inst;
   register mqword_t reg1 = core->registers[(curr >> 4) & 15];
   register mqword_t reg2 = core->registers[curr & 15];
   core->registers[(curr >> 4) & 15] &= (0xFFFFFFFFFFFFFF00 | (reg2 & 0xFFFF));
   core->registers[curr & 15] &= (0xFFFFFFFFFFFFFF00 | (reg1 & 0xFFFF));
}

_MERRY_ALWAYS_INLINE_ _exec_(excg32)
{
   // just exchange the bytes and don't overwrite anything
   register mqword_t curr = core->current_inst;
   register mqword_t reg1 = core->registers[(curr >> 4) & 15];
   register mqword_t reg2 = core->registers[curr & 15];
   core->registers[(curr >> 4) & 15] &= (0xFFFFFFFFFFFFFF00 | (reg2 & 0xFFFFFF));
   core->registers[curr & 15] &= (0xFFFFFFFFFFFFFF00 | (reg1 & 0xFFFFFF));
}

_MERRY_ALWAYS_INLINE_ _exec_(excg)
{
   // just exchange the bytes and don't overwrite anything
   register mqword_t curr = core->current_inst;
   register mqword_t reg1 = core->registers[(curr >> 4) & 15];
   register mqword_t reg2 = core->registers[curr & 15];
   core->registers[(curr >> 4) & 15] = reg2;
   core->registers[curr & 15] = reg1;
}

_MERRY_ALWAYS_INLINE_ _lexec_(atm_load, mqword_t address)
{
   // read from the given address
   if (merry_dmemory_read_qword_atm(core->data_mem, address, &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be loaded
}

_MERRY_ALWAYS_INLINE_ _lexec_(atm_store, mqword_t address)
{
   // store to the given address from the given register
   if (merry_dmemory_write_qword_atm(core->data_mem, address, core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be stored
}

_MERRY_ALWAYS_INLINE_ _lexec_(atm_loadw, mqword_t address)
{
   // read from the given address
   if (merry_dmemory_read_word_atm(core->data_mem, address, &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be loaded
}

_MERRY_ALWAYS_INLINE_ _lexec_(atm_storew, mqword_t address)
{
   // store to the given address from the given register
   if (merry_dmemory_write_word_atm(core->data_mem, address, core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be stored
}

_MERRY_ALWAYS_INLINE_ _lexec_(atm_loadd, mqword_t address)
{
   // read from the given address
   if (merry_dmemory_read_dword_atm(core->data_mem, address, &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be loaded
}

_MERRY_ALWAYS_INLINE_ _lexec_(atm_stored, mqword_t address)
{
   // store to the given address from the given register
   if (merry_dmemory_write_dword_atm(core->data_mem, address, core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be stored
}

_MERRY_ALWAYS_INLINE_ _lexec_(atm_loadb, mqword_t address)
{
   // read from the given address
   if (merry_dmemory_read_byte_atm(core->data_mem, address, &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be loaded
}

_MERRY_ALWAYS_INLINE_ _lexec_(atm_storeb, mqword_t address)
{
   // store to the given address from the given register
   if (merry_dmemory_write_byte_atm(core->data_mem, address, core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE)
   {
      merry_requestHdlr_panic(core->data_mem->error);
      core->stop_running = mtrue;
      return; // failure
   }
   // the value should be stored
}