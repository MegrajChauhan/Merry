;; The 'broadcast' should behave as expected, however, the 'signal' won't behave as expected when
;; there are multiple threads waiting on the same condition variable. Once a 'signal' is issued,
;; every thread will get the signal but the behavior expects only one thread to continue while other
;; threads continue waiting. To achieve this, we introduce a condition lock.
;; Every condition variable is now one byte lock and one byte condition variable.
;; This makes a condition variable a data structure rather than a simple variable. 

depends _builtinmlocks_.asm

proc __builtin_std_raw_condition_variable_wait
proc __builtin_std_raw_condition_variable_signal
proc __builtin_std_raw_condition_variable_broadcast

;; ARGS: Ma = PTR to condition variable, Mb = PTR to a lock
;; NOTE: There are no safety checks and it will not return anything as well
__builtin_std_raw_condition_variable_wait
    excgq Ma, Mb
    call __builtin_std_raw_release

    mov Mc, Mb
    sub Mc, _MSTD_COND_VAR_GET_LOCK_
    loadb Mc, Mc                       ;; get the condition lock
    
 _std_raw_condition_variable_loop
    atm loadb M1, Mb
    cmp M1, _MSTD_COND_VAR_SIGNAL_
    je _std_raw_condition_variable_signaled
    
    cmp M1, _MSTD_COND_VAR_BROADCAST_
    je _std_raw_condition_variable_broadcasted

    jmp _std_raw_condition_variable_loop

 _std_raw_condition_variable_signaled
    excg Ma, Mc                       ;; since Mc had the condition lock
    call __builtin_std_raw_acquire    ;; acquire the condition lock

    atm loadb M1, Mb                  ;; get the condition variable's state again
    cmp M1. _MSTD_COND_VAR_NONE_
    
    je _std_raw_condition_variable_already_signaled ;; a bummer!

    jmp _std_raw_condition_variable_signal_done

_std_raw_condition_variable_already_signaled
    call __builtin_std_raw_release       ;; release the condition lock
    jmp _std_raw_condition_variable_loop ;; one thread already took the signal

_std_raw_condition_variable_signal_done
    movl Mc, _MSTD_COND_VAR_NONE_
    atm storeb Mc, Mb 
    call __builtin_std_raw_release       ;; release the condition lock
    call __builtin_std_raw_acquire       ;; One thread will get the lock back
    ret 

 _std_raw_condition_variable_broadcasted
    movl Mc, _MSTD_COND_VAR_NONE_
    atm storeb Mc, Mb 
    ret                               ;; The broadcasting thread has to release the lock

;; ARGS: Ma = PTR to the condition variable
;; NOTE: There are no safety checks and it will not return anything as well
__builtin_std_raw_condition_variable_signal
    movl Mb, _MSTD_COND_VAR_SIGNAL_
    atm storeb Mb, Ma
    ret

;; ARGS: Ma = PTR to the condition variable
;; NOTE: There are no safety checks and it will not return anything as well
__builtin_std_raw_condition_variable_broadcast
    movl Mb, _MSTD_COND_VAR_BROADCAST_
    atm storeb Mb, Ma
    ret

dc _MSTD_COND_VAR_NONE_      0
dc _MSTD_COND_VAR_SIGNAL_    1
dc _MSTD_COND_VAR_BROADCAST_ 2
dc _MSTD_COND_VAR_GET_LOCK_  1