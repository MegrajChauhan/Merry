#ifndef _MERRY_ERRORS_
#define _MERRY_ERRORS_

// this includes all of the internal errors that may be produced

/*It is just sad to see that for something as powerful as a decoder, the program has to give up on its ability to call as many function as it desires.*/
/*The call depth will be set according to how much the stack can handle*/
enum
{
    MERRY_ERROR_NONE,
    _PANIC_REQBUFFEROVERFLOW = 1,
    _PANIC_DECODER_NOT_STARTING,
    MERRY_MEM_ACCESS_ERROR = 51,   // accessing the memory in a wrong way
    MERRY_MEM_INVALID_ACCESS,      // indicating memory access for memory addresses that either do not exist or are invalid
    MERRY_DIV_BY_ZERO,             // dividing by zero
    MERRY_STACK_OVERFLOW,          // stack is overflowing
    MERRY_STACK_UNDERFLOW,         // the stack has less than expected number of values
    MERRY_CALL_DEPTH_REACHED,      // reached the limit of function calls
    MERRY_INVALID_RETURN,          // invalid return instruction
    MERRY_INVALID_VARIABLE_ACCESS, // accessing variable in the wrong way on the stack
};

#endif