depends stdfio.asm

;; We don't need stdinit or stdmem

entry main

proc main

main
    mov Ma, [PTR FILENAME]
    movl Mb, _M_WRITE_
    or Mb, _M_CREATE_
    movl Mc, _M_CREATE_RWX
    call std::fio::fopen
    call std::fio::fclose
    halt

ds FILENAME "Test.txt".0