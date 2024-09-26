depends stdinit.asm
depends stdmem.asm
depends stdmt.asm
depends stdsync.asm
depends stdcont.asm
depends stddefs.asm

proc main2
proc main
proc print

entry main

main
    call std::init
    call std::mem::init_allocator
    movl Mm5, 0
    mov Ma, 8
    mov Mb, 1
    mov Mc, 1 ;; no custom allocator
    call std::cont::create
    cmp Ma, NULL
    je _error
    storeq Ma, cont

    ;; new thread
    push Ma
    movl Ma, main2
    call std::mt::spawn
    cmp Ma, 1
    je _error ;; we won't free it :)
    pop Ma

    ;; pushing
    mov Mb, [PTR to_push1]
    call std::cont::push
    cmp Ma, NULL
    je _error
    mov M1, [PTR pushing]
    cout M2
    call print
    ;;intr _M_BP_

    ;; try to get information now
    movl Mc, 1000
 _tmp
    loop _tmp
    push Ma
    call std::cont::csize
    uoutq Ma
    mov Ma, 10
    cout Ma
    pop Ma    

    ;; pop now
    ;; update info(just in case)
    push Ma
    call std::cont::csize
    uoutq Ma
    mov Mc, Ma
    mov Ma, 10
    cout Ma
    pop Ma
    dec Mc
    intr _M_BP_
    intr _M_BP_
    intr _M_BP_
 _loop
    movl Mb, [PTR temp]
    call std::cont::pop
    cmp Ma, NULL
    je _error
    loadq Mb, temp
    uoutq Mb
    loop _loop
    
    call std::cont::destroy
    movl Ma, 10
    cout Ma
    xor Ma, Ma
    halt
 _error
    movl Ma, 1
    intr _M_EXIT_

main2
    loadq Ma, cont
    movl Mm5, 1

    ;; pushing
    mov Mb, [PTR to_push2]
    call std::cont::push
    cmp Ma, NULL
    je _error
    mov M1, [PTR pushing]
    call print
    
    ;; resize
    mov Mb, 2
    call std::cont::resize
    cmp Ma, NULL
    je _error
    mov M1, [PTR resizing]
    call print
    
    ;; capacity
    push Ma
    call std::cont::ccapacity
    uoutq Ma
    mov Ma, 10
    cout Ma
    pop Ma
    
    mov Mb, 0
    mov Mc, [PTR to_push2]
    call std::cont::insert
    cmp Ma, NULL
    je _error
    mov M1, [PTR inserting]
    call print
    intr _M_BP_
    
    xor Ma, Ma
    halt

print
    uoutq Mm5
    push Mc
    movl Mc, 8
    sout M1
    movl Mc, 10
    cout Mc
    pop Mc
    ret

rq temp 1
rq cont 1

ds pushing "Pushing".0
ds popping "Popping".0
ds resizing "resizing".0
ds inserting "insertin".0

dq to_push1 0xAABBCCDDEEFF0123
dq to_push2 0xC0FFEE1510101010
