depends _builtinstdstate_.asm

proc __lib_init
proc __lib_is_threaded
proc __lib_is_multi_process
proc __lib_uses_builtin_alloc
proc __lib_builtin_alloc_init
proc __lib_uses_subsys
proc __lib_subsys_init
proc __lib_uses_dynlib
proc __lib_dynlib_init

__lib_init
    xor Ma, Ma
    storeb Ma, _Mstd_multi_threaded
    storeb Ma, _Mstd_multi_process
    storeb Ma, _Mstd_allocator_builtin
    storeb Ma, _Mstd_allocator_init
    storeb Ma, _Mstd_subsys_init
    storeb Ma, _Mstd_subsys_any
    storeb Ma, _Mstd_dyn_init
    storeb Ma, _Mstd_dyn_any
    inc Ma
    storeb Ma, _Mstd_stdlib_init
    ret

__lib_is_threaded
    movl Ma, 1
    storeb Ma, _Mstd_multi_threaded
    ret

__lib_is_multi_process
    movl Ma, 1
    storeb Ma, _Mstd_multi_process
    ret

__lib_uses_builtin_alloc
    movl Ma, 1
    storeb Ma, _Mstd_allocator_builtin
    ret

__lib_builtin_alloc_init
    movl Ma, 1
    storeb Ma, _Mstd_allocator_init
    ret

__lib_uses_subsys
    movl Ma, 1
    storeb Ma, _Mstd_subsys_any
    ret

__lib_subsys_init
    movl Ma, 1
    storeb Ma, _Mstd_subsys_init
    ret

__lib_uses_dynlib
    movl Ma, 1
    storeb Ma, _Mstd_dyn_init
    ret

__lib_dynlib_any
    movl Ma, 1
    storeb Ma, _Mstd_dyn_any
    ret