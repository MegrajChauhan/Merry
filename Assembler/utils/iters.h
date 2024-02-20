#ifndef _ITERS_
#define _ITERS_

typedef struct Iter
{
    void *start, *end;
} Iter;

// have the iterators reached the same point
#define _ITER_SAME_(iter) (iter.start == iter.end)

#endif