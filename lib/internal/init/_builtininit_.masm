proc __builtin_std_init
proc __builtin_std_get_cmd_options

;; Takes no arguments
__builtin_std_init
    ;; Store everything
    storeb Mm1, _Mstd_option_count
    storeb Mm2, _Mstd_num_of_pages
    storeb Mm3, _Mstd_host_os_id
    storeb Mm4, _Mstd_host_arch_id
    storeq Md, _Mstd_option_start_addr
    storeq Mm5, _Mstd_free_mem_start_addr

    ;; What do we have?
    cmp Mm1, 1
    js _std_ret
    mov Ma, _MSTD_NULL_
    storeq Ma, _Mstd_option_start_addr

_std_ret
    ret

;; __builtin_std_init is the first procedure that must be called to use the STDLIB
;; For proper behavior, call before starting anything

;; ARGS: Ma = PTR arg_count, Mb = PTR arg_value
;; RETURN = Nothing, check Ma == 1
__builtin_std_get_cmd_options
    call __builtin_quick_save
    loadb M1, _Mstd_option_count
    loadq M2, _Mstd_option_start_addr
    storeb M1, Ma
    storeq M2, Mb
    call __builtin_quick_restore
    ret

;; Firstly, the initialization data
;; The data

rb _Mstd_option_count 1 ;; Reserve 1 byte to store the number of options
rb _Mstd_host_arch_id 1 ;; The VM provides these IDs
rb _Mstd_host_os_id 1   ;; This is also provided by the VM
rb _Mstd_num_of_pages 1 ;; The number of pages that the program can currently use

rb _Mstd_reserve 4 ;; reserve for future use

rq _Mstd_option_start_addr 1   ;; So that we can access the options
rq _Mstd_free_mem_start_addr 1 ;; For memory allocation purposes
