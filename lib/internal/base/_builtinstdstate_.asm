;; Every single variable that determines the stdlib's state

rb _Mstd_multi_threaded 1 ;; Has any threads been created?
rb _Mstd_allocator_builtin 1 ;; Was the builtin allocator used?
rb _Mstd_multi_process 1 ;; Any new processes?
rb _Mstd_stdlib_init 1 ;; Has the stdlib been initialized?