.intel_syntax noprefix
#include <merry_config.h>
#include <merry_utils.h>

// We believe that every machine that Merry runs on uses 2's complement for negative number representation and
// IEEE representation for floating point numbers

// void _update_flags_(MerryFlagRegister *reg)
.global _update_flags_
// void _cmp_inst_(mqword_t val1, mqword_t val2, struct MerryFlagRegsiter *reg)
.global _cmp_inst_


#if defined(_MERRY_HOST_CPU_x86_64_ARCH_)
_update_flags_:
   pushfq          // push the flags register unto the stack
   pop rax        // pop the flags
   mov [rdi], rax // save the values back such that we have the flags value
   ret

#if defined(_USE_LINUX_)
_cmp_inst_:
   cmp rdi, rsi
   mov rdi, rdx
   call _update_flags_
   ret
#endif

#if defined(_USE_WIN_)
_cmp_inst_:
   cmp rcx, rdx
   mov rdi, r8
   call _update_flags_
   ret
#endif

#endif