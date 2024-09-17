proc __builtin_get_errno
proc __builtin_set_errno

;; ARGS: Ma = errno value
__builtin_set_errno
    storeb Ma, _Mstd_errno
    ret

;; RETURNS: Ma = errno value
__builtin_get_errno
    loadb Ma, _Mstd_errno
    ret

db _Mstd_errno 1 ;; just one