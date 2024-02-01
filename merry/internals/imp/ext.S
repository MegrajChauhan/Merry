// since it is the case for every architecture, we simply implement platform independent code here

// mqword_t _inc_inst_(mqword_t _to_inc);
.global _inc_inst_
// mqword_t _dec_inst_(mqword_t _to_dec);
.global _dec_inst_

_inc_inst_:
   mov rax, rdi
   inc rax
   ret

_dec_inst_:
   mov rax, rdi
   dec rax
   ret
   