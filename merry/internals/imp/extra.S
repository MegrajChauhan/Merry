.intel_syntax noprefix

#include "../../../utils/merry_config.h"

.global _mem_read_qword_
.global _mem_read_word_
.global _mem_read_dword_

#if defined(_MERRY_HOST_OS_LINUX_)

#if defined(_MERRY_HOST_CPU_x86_64_ARCH_)
// Read a word (2 bytes) from memory
_mem_read_word_:
    movzx rax, word ptr [rdi]  // Load a word (2 bytes) from the source address into rax
    mov [rsi], ax               // Store the word to the destination address
    ret

// Read a dword (4 bytes) from memory
_mem_read_dword_:
    mov eax, dword ptr [rdi]    // Load a dword (4 bytes) from the source address into eax
    mov [rsi], eax              // Store the dword to the destination address
    ret

// Read a qword (8 bytes) from memory
_mem_read_qword_:
    mov rax, qword ptr [rdi]    // Load a qword (8 bytes) from the source address into rax
    mov [rsi], rax              // Store the qword to the destination address
    ret
#endif

#endif