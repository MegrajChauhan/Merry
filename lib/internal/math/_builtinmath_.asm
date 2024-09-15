;; With the current state of the VM, the library strictly requires the support for floating point numbers
;; The assembler also needs modifications to understand numbers such as '1e-123'
;; Writing a math support might just be the extreme for me


dc _MSTD_PI_NUM_ 3.141592653589793238462643383279502884197
dc _MSTD_EULER_NUM_ 2.7182818284590452353602874713526624977572470936999595 ;; The assembler will only use what it can i.e what C++ allows

dc _MSTD_POSITIVE_INF32_ 0b01111111100000000000000000000000
dc _MSTD_NEGATIVE_INF32_ 0b11111111100000000000000000000000

;; Is it just me or is IEEE754 that hard to understand?