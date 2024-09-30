depends stdutils.asm

proc main
entry main

main
    intr 165
    movl Ma, [PTR subsysname]
    intr 166
    storeq Ma, subsysid
    movl Mb, 2 ;; request
    movl M1, 10
    movl M2, 20
    movl M3, 30
    movl M4, 40
    intr 169
    ;; result should be in Ma
    uoutq Ma
    loadq Ma, subsysid
    intr 167 ;; close the subsystem
 _exit
    movl Ma, 0
    call std::util::exit

rq subsysid 1 ;; to hold the channel id
ds subsysname "./subsys.so".0