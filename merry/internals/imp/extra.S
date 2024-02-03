.intel_syntax noprefix

// void _mem_read_(<Any Pointer> a, <Any pointer> b)
.global _mem_read_

// This is used to read 8 bytes at once from the memory
_mem_read_:
    movq rax, [rdi]
    movq [rsi], rax
    ret