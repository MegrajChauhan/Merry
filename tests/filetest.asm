depends stdfio.asm

;; We don't need stdinit or stdmem

entry main

proc main

main
    mov Ma, [PTR FILENAME]
    movl Mb, _M_CREATE_
    or Mb, _M_READ_WRITE_
    movl Mc, _M_CREATE_RWX
    call std::fio::fopen
    call std::fio::fclose
    movl Ma, 0
    halt

ds FILENAME "Test.txt".0