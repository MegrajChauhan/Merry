depends stdsync.asm
depends stdmem.asm
depends stdinit.asm
depends stdutils.asm

proc main1
proc main2

main1
    call std::init
    call std::mem::init_allocator
    mov Ma, [PTR _one_lock]
    call std::sync::mutex_init
    
    movl Mc, 20
    movl Md, 10
    jmp _alloc_loop

 _alloc_loop
    call std::sync::mutex_lock
    push Ma
    movl Ma, 32
    call std::mem::alloc
    uoutq Mc
    cout Md
    pop Ma
    call std::sync::mutex_unlock
    loop _alloc_loop
    mov Ma, 0
    call std::util::exit

main2
    movl Mc, 2000
_l
    loop _l
    movl Mc, 20
    movl Md, 10
    jmp _alloc_loop
    
entry main1
entry main2

rb _one_lock 1 ;; Our lock 