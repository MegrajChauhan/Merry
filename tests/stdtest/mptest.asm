depends stdmp.asm
depends stdutils.asm

proc _entry
entry _entry

_entry
    cmp Ma, 1
    je _already_spawned
    call std::mp::spawn ;; spawn a new process
 _parent_proc
    ;; in parent process
    movl Mc, 25
    mov Ma, [PTR parent]
    sout Ma
    jmp _exit 
 _already_spawned
    ;; in child process
    
    movl Mc, 24
    mov Ma, [PTR child]
    sout Ma
 _exit
    mov Ma, 0
    call std::util::exit

ds parent "Hello World from parent!".10.0
ds child "Hello World from child!".10.0