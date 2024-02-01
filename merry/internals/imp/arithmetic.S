.intel_syntax noprefix
// This implements the arithmetic instructions at the hardware level
#include "../../../utils/merry_config.h"

// We believe that every machine that Merry runs on uses 2's complement for negative number representation and
// IEEE representation for floating point numbers

// void _update_flags_(MerryFlagRegister *reg)
.global _update_flags_

// mqword_t _add_inst_(mqword_t op1, mqword_t op2)
.global _add_inst_
// mqword_t _sub_inst_(mqword_t op1, mqword_t op2)
.global _sub_inst_
// mqword_t _mul_inst_(mqword_t op1, mqword_t op2)
.global _mul_inst_
// mqword_t _div_inst_(mqword_t op1, mqword_t op2)
.global _div_inst_
// mqword_t _mod_inst_(mqword_t op1, mqword_t op2)
.global _mod_inst_

// mqword_t _iadd_inst_(mqword_t op1, mqword_t op2)
.global _iadd_inst_
// mqword_t _isub_inst_(mqword_t op1, mqword_t op2)
.global _isub_inst_
// mqword_t _imul_inst_(mqword_t op1, mqword_t op2)
.global _imul_inst_
// mqword_t _idiv_inst_(mqword_t op1, mqword_t op2)
.global _idiv_inst_
// mqword_t _imod_inst_(mqword_t op1, mqword_t op2)
.global _imod_inst_
// void _cmp_inst_(mqword_t val1, mqword_t val2, MerryFlagRegister *reg)
.global _cmp_inst_


#if defined(_MERRY_HOST_OS_LINUX_)
_add_inst_:
    mov rax, rdi  // get the first operand
    add rax, rsi  // add the second operand
    ret            // return the result in rax 

// same as add above but for other instructions
_sub_inst_:
    mov rax, rdi  // get the first operand
    sub rax, rsi  // sub the second operand
    ret            // return the result in rax 

_mul_inst_:
    mov rax, rdi  
    mul rax, rsi  
    ret
     
_div_inst_:
    mov rax, rdi  
    div rax, rsi  
    ret
     
_mod_inst_:
    mov rax, rdi  
    mod rax, rsi  
    ret

// same as unsigned
_iadd_inst_:
    mov rax, rdi  
    iadd rax, rsi 
    ret 

_isub_inst_:
    mov rax, rdi 
    isub rax, rsi
    ret 

_imul_inst_:
    mov rax, rdi  
    imul rax, rsi  
    ret
     
_idiv_inst_:
    mov rax, rdi  
    idiv rax, rsi  
    ret
     
_imod_inst_:
    mov rax, rdi  
    imod rax, rsi  
    ret

_cmp_inst_:
    cmp rdi, rsi // since these registers contain the value
    mov rdi, rdx // the flags register 
    call _update_flags_
    ret
#endif

#if defined(_MERRY_HOST_CPU_AMD_)
_update_flags_:
   pushfq          // push the flags register unto the stack
   pop rax        // pop the flags
   mov [rdi], rax // save the values back such that we have the flags value
   ret
#endif