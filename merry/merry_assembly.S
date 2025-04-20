.intel_syntax noprefix
#include "merry_config.h"

// We believe that every machine that Merry runs on uses 2's complement for negative number representation and
// IEEE representation for floating point numbers

// void merry_update_flags_regr(MerryFlagRegr *reg)
.global merry_update_flags_regr

// void merry_compare_two_values(mqword_t val1, mqword_t val2, struct MerryFlagRegr *reg)
.global merry_compare_two_values


#if defined(_MERRY_HOST_CPU_x86_64_ARCH_)
merry_update_flags_regr:
   pushfq          // push the flags register unto the stack
   pop rax        // pop the flags
   mov [rdi], rax // save the values back such that we have the flags value
   ret

#if defined(_USE_LINUX_)
merry_compare_two_values:
   cmp rdi, rsi
   mov rdi, rdx
   call merry_update_flags_regr 
   ret
#endif

#if defined(_USE_WIN_)
merry_compare_two_values:
   cmp rcx, rdx
   mov rdi, r8
   call merry_update_flags_regr
   ret
#endif

#endif
