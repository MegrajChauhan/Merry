depends _builtindefs_.asm

proc __builtin_std_raw_acquire
proc __builtin_std_raw_release

;; ARGS: Ma = PTR to the mutex lock[Must be a byte]
;; RETURNS: Nothing
;; NOTE: This doesn't save states so save it yourself
__builtin_std_raw_acquire
    movl M1, _MSTD_RELEASE_         ;; expected
    movl M2, _MSTD_ACQUIRE_         ;; desired
    cmpxchg M1, M2, Ma              ;; I apologize for the extremely weird syntax[Please look at asm/tests/masm/test.masm line 228 for details] 
    jne __builtin_std_raw_acquire   ;; keep trying until unlocked
    ret

;; ARGS: Ma = PTR to the mutex lock
;; NOTE: This doesn't save states so save it yourself
__builtin_std_raw_release
    movl Mb, _MSTD_RELEASE_
    storeq Mb, Ma
    ret

dc _MSTD_ACQUIRE_ 1
dc _MSTD_RELEASE_ 0