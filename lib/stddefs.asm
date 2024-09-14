depends _builtindefs_.asm

defined _M_LINUX_
;; constants related to linux
dc __linux__ 1
end

defined _M_AMD_
dc __x86_64__ 1
dc __amd__ 1
end

dc NULL [_MSTD_NULL_]
dc true 1
dc false 0

dc SUCCESS [true]
dc FAILURE [false]

dc __stdlibv__ 00010  ;; [MAJOR][MINOR][PATCH][RELEASE NUMBER]

