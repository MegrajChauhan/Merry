#include "merry_ihdlr.h"

// handle the signal to terminate
merry_ihdlr(termianation)
{
   // The OS knows that this isn't from any process
   merry_requestHdlr_panic(_SHOULD_EXIT, 0);
}

// we have been interrupted
// We do not do anything in this case
// We may as well say that we 'ignore' this signal
merry_ihdlr(interrupt)
{
}

// A segmentation fault
// either in imem or dmem
merry_ihdlr(segv)
{
   merry_requestHdlr_panic(MERRY_SEGV, 0);
}

/**
 * With the segmentation fault, the most likely cause is the syscall instruction.
 * The number of sources are enormous but i believe this one to be the most likely.
 * Say we have a syscall that is trying to open a file.
 * In any OS, the syscall requires the passing of pointers to a C-style string.
 * To the program, the pointer may be completely valid and it should be but due to how
 * we have the memories implemented, it is compulsory for the entire string to be on the same page.
 * This applies to other data types too. This is a pain but it is one of the sacrifices made to
 * squeeze out speed for the VM. Every check done everytime a memory address is accessed is truly a pain.
 * The only solution to this is to let the programmer freely write the program without having to worry about it
 * and handle the alignment in the assembler. This is a viable option but it makes the binary bigger.
 * I believe that that sacrifice is worth it BUT the next problem lies with dynamic allocations.
 * A running program might allocate some memory but the assembler won't be able to do anything about it.
 * This also means that the library will have to make sure that everything is aligned.
 * This is all a pain and I hope to solve it sometime.
 * The only most feasible solution that I have for this problem is the limited use and power of syscall.
 * The syscall instruction may be passed to the OS for handling.
 * This was we can limit the programs from making certain calls and make sure that
 * those calls are harmless. The only flipside to this being the implementation details.
 */