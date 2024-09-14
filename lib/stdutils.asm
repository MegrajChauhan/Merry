depends _builtinutils_.asm

proc std::util::exit

;; ARGS: Ma = Return value
std::exit::exit
    call __builtin_exit