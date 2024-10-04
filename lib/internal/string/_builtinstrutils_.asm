;; All utility procedures for strings

depends _builtindefs_.asm
depends _builtinalloc_.asm

proc __builtin_std_strlen
proc __builtin_std_strcmp
proc __builtin_std_strcpy
proc __builtin_std_strncpy
proc __builtin_std_strncmp
proc __builtin_std_starts_with
proc __builtin_std_ends_with
proc __builtin_std_substr
proc __builtin_std_strfind
proc __builtin_std_strcat
proc __builtin_std_strncat
proc __builtin_std_toUpper
proc __builtin_std_toLower

;; ARGS: Ma = PTR to a c-string
;; RETURNS: Ma = len else -1 for error
__builtin_std_strlen
    push _MSTD_NULL_
    pusha
    cmp Ma, _MSTD_NULL_
    sss 
    je _std_strlen_done
    
    xor M1, M1 ;; M1 is the counter
    ;; Loop until we find the len
 _std_strlen_loop
    loadb Mb, Ma
    cmp Mb, 0
    je _std_strlen_loop_done ;; done
    inc Ma
    inc M1
    jmp _std_strlen_loop
 
 _std_strlen_loop_done
    sss M1, 1 ;; set the return value
 _std_strlen_done
    popa
    pop Ma
    ret

;; ARGS: Ma = PTR to first c-string, Mb = PTR to second c-string
;; RETURN: 0 for equal(and same), 1 for not(may be equal), -1 for error 
__builtin_std_strcmp
    push _MSTD_NULL_
    pusha
    cmp Ma, _MSTD_NULL_
    cmp Mb, _MSTD_NULL_
    je _std_strcmp_done
    
    movl Mm1, 1

    push Ma
    call __builtin_std_strlen ;; won't fail
    mov M1, Ma ;; save the length
    push Mb
    excgq Ma, Mb
    call __builtin_std_strlen
    mov M2, Ma ;; second string's length
    pop Mb
    pop Ma

    ;; M1 and M2 now have the lengths
    cmp M1, M2 
    sss Mm1, 1 ;; return value
    jne _std_strcmp_done
    mov Mc, M1 ;; whichever will do
 _std_strcmp_loop
    loadb M1, Ma
    loadb M2, Mb
    cmp M1, M2
    jne _std_strcmp_done
    inc Ma
    inc Mb
    loop _std_strcmp_loop
    
    dec Mm1
    sss Mm1, 1
 _std_strcmp_done
    popa
    pop Ma
    ret

;; ARGS: Ma = SRC, Mb = DEST
;; RETURNS: Ma = Mb else NULL for error
__builtin_std_strcpy
   push Ma
   cmp Ma, _MSTD_NULL_
   je _std_strcpy_done
   call __builtin_std_strlen
   mov Mc, Ma ;; the number of bytes
   pop Ma
   call __builtin_std_memscpy
 _std_strcpy_done
   ret

;; ARGS: Ma = SRC, Mb = DEST, Mc = Number of bytes to copy
;; RETURNS: Ma = Mb else NULL for error
__builtin_std_strncpy
   call __builtin_std_memscpy
   ret

;; ARGS: Ma = PTR to first c-string, Mb = PTR to second c-string, Mc = number of characters to compare
;; RETURN: 0 for equal(and same), 1 for not(may be equal), -1 for error 
__builtin_std_strncmp
    push _MSTD_NULL_
    pusha
    cmp Ma, _MSTD_NULL_
    cmp Mb, _MSTD_NULL_
    je _std_strncmp_done
    
    movl Mm1, 1

    push Ma
    call __builtin_std_strlen ;; won't fail
    mov M1, Ma ;; save the length
    push Mb
    excgq Ma, Mb
    call __builtin_std_strlen
    mov M2, Ma ;; second string's length
    pop Mb
    pop Ma

    ;; M1 and M2 now have the lengths
    cmp M1, Mc
    cmp M2, Mc
    jge _std_strncmp_loop
    
    ;; The length is greater than the length of either strings
    movl Mm1, _MSTD_NULL_
    sss Mm1, 1
    jmp _std_strncmp_done
 _std_strncmp_loop
    loadb M1, Ma
    loadb M2, Mb
    cmp M1, M2
    jne _std_strncmp_done
    inc Ma
    inc Mb
    loop _std_strncmp_loop
    
    dec Mm1
    sss Mm1, 1
 _std_strncmp_done
    popa
    pop Ma
    ret

;; ARGS: Ma = PTR to the c-string to check, Mb = PTR to the c-string to look at
;; RETURNS: Ma = 1 for success else 0 or NULL for error
__builtin_std_starts_with
   push _MSTD_NULL_
   pusha
   cmp Ma, _MSTD_NULL_
   cmp Mb, _MSTD_NULL_
   je _std_starts_with_done

   push Ma
   call __builtin_std_strlen
   mov M1, Ma
   push Mb
   mov Ma, Mb
   call __builtin_std_strlen
   mov M2, Ma
   pop Mb
   pop Ma

   cmp M1, M2
   jg _std_starts_with_done ;; error
   
   xor Mm1, Mm1
   sss Mm1, 1
   mov Mc, M1
 _std_starts_with_loop
   loadb Mm1, Ma
   loadb Mm2, Mb
   cmp Mm1, Mm2
   jne _std_starts_with_done
   inc Ma
   inc Mb
   loop _std_starts_with_loop
   
   movl Mm1, 1
   sss Mm1, 1
 _std_starts_with_done
   popa
   pop Ma
   ret

;; ARGS: Ma = PTR to the c-string to check, Mb = PTR to the c-string to look at
;; RETURNS: Ma = 1 for success else 0 or NULL for error
__builtin_std_ends_with
   push _MSTD_NULL_
   pusha
   cmp Ma, _MSTD_NULL_
   cmp Mb, _MSTD_NULL_
   je _std_ends_with_done

   push Ma
   call __builtin_std_strlen
   mov M1, Ma
   push Mb
   mov Ma, Mb
   call __builtin_std_strlen
   mov M2, Ma
   pop Mb
   pop Ma

   cmp M1, M2
   jg _std_ends_with_done ;; error
   
   sub M2, M1 ;; to get the offset
   add Ma, M2 ;; correct offset 

   xor Mm1, Mm1
   sss Mm1, 1
   mov Mc, M1
 _std_ends_with_loop
   loadb Mm1, Ma
   loadb Mm2, Mb
   cmp Mm1, Mm2
   jne _std_ends_with_done
   inc Ma
   inc Mb
   loop _std_ends_with_loop
   
   movl Mm1, 1
   sss Mm1, 1
 _std_ends_with_done
   popa
   pop Ma
   ret

;; ARGS: Ma = PTR to the string, Mb = index start, Mc = index end
__builtin_std_substr