dc _M_HALT_ 151      ;; Halts the current core(same behavior as the halt instruction)
dc _M_EXIT_ 152      ;; Kill all threads and exit
dc _M_NEW_CORE_ 153  ;; Add a new vcore 
dc _M_MEM_ 154       ;; Get more memory from the Host OS
dc _M_NEW_PROC_ 155  ;; Start a new procedure
dc _M_RES1_ 156      ;; For debuggers(Any program shouldn't use this)
dc _M_BP_ 157        ;; For debuggers as well(Acts as a breakpoint)
dc _M_RES2_ 158      ;; For debuggers only(should't be used)
dc _M_LOAD_LIB 159   ;; load a new library
dc _M_UNLOAD_LIB 160 ;; unload a new library
dc _M_GET_FUNC 161   ;; get a function from a loaded library
dc _M_SYSCALL 162    ;; make a syscall