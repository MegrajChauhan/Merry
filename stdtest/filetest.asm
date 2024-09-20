depends stdfio.asm

;; We don't need stdinit or stdmem

entry main

proc main

main
    mov Ma, [PTR FILENAME]
    movl Mb, _M_READ_WRITE_
    ;; or Mb, _M_READ_WRITE_
    ;; movl Mc, _M_CREATE_RWX
    call std::fio::fopen

    cmp Ma, ERR
    je _exit
    
    ;; Ma already has the fd
    ;; but store it
    storeq Ma, fd


    mov Mb, [PTR _output_text_]
    mov Mc, _output_text_len_
    call std::fio::fwrite

    cmp Ma, ERR
    je _exit

    loadq Ma, fd
    call std::fio::fclose
    movl Ma, 0

_exit
    halt

ds FILENAME "Test.txt".0
dc ERR 0xFFFFFFFFFFFFFFFF

ds _output_text_ "Hello World!".10.0
dc _output_text_len_ 12

dq fd 1