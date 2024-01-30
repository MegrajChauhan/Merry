.intel_syntax noprefix
// This implements the arithmetic instructions at the hardware level
#include "../../../utils/merry_config.h"

// We believe that every machine that Merry runs on uses 2's complement for negative number representation and
// IEEE representation for floating point numbers

// void _update_flags_(MerryFlagRegister *reg)
.global _update_flags_

// mqword_t _add_inst_(mqword_t op1, mqword_t op2)
.global _add_inst_

#if defined(_MERRY_HOST_OS_LINUX_)
_add_inst_:
    mov rax, rdi  // get the first operand
    add rax, rsi  // add the second operand
    ret            // return the result in rax 
#endif

#if defined(_MERRY_HOST_CPU_AMD_)
_update_flags_:
   pushfq          // push the flags register unto the stack
   pop rax        // pop the flags
   mov [rdi], rax // save the values back such that we have the flags value 
   ret
#endif