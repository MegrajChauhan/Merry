depends _builtininit_.asm

proc std::init
proc std::cmd_options

std::init
    call __builtin_std_init
    ret

;; ARGS: Ma = PTR arg_count, Mb = PTR arg_value
std::cmd_options
    call __builtin_std_get_cmd_options
    ret