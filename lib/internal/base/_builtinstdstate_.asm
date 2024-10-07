;; Every single variable that determines the stdlib's state

rb _Mstd_multi_threaded 1    ;; Has any threads been created?
rb _Mstd_allocator_builtin 1 ;; Was the builtin allocator used(if used other allocators may not be used)?
rb _Mstd_multi_process 1     ;; Any new processes?
rb _Mstd_stdlib_init 1       ;; Has the stdlib been initialized?
rb _Mstd_allocator_init 1    ;; Allocator initialized?
rb _Mstd_subsys_init 1       ;; Was the subsystem initialized?
rb _Mstd_subsys_any 1        ;; any subsystem were started at any point?
rb _Mstd_dyn_init 1          ;; Was the dynamic library used?
rb _Mstd_dyn_any 1           ;; Any dynamic library loaded?
