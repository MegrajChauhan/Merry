.intel_syntax noprefix

.global read
.global get

read:
  movq rax, [rdi]
  movq [rsi], rax
  ret 

get:
 cmp rdi, rsi
 pushfq
 pop rax
 mov [rdx], rax
 ret