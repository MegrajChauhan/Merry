#if defined(_WIN64)
#include "..\utils\merry_stack.h"
#else
#include "../utils/merry_stack.h"
#endif

MerryStack *merry_init_stack(msize_t len, mbool_t dynamic, msize_t upper_lim, msize_t per_resize)
{
    MerryStack *st = (MerryStack *)malloc(sizeof(MerryStack));
    if (st == NULL)
        return RET_NULL;
    st->array = (mqptr_t)malloc(8 * len);
    if (st->array == NULL)
    {
        free(st);
        return RET_NULL;
    }
    st->size = len;
    st->sp = 0;
    st->dynamic = dynamic;
    st->add_per_resize = per_resize;
    st->upper_lim = (upper_lim == 0) ? 1000 : upper_lim; // upper limit == 0 means no limit but since we want it to be limited, 1000 is the limit
    return st;
}

_MERRY_ALWAYS_INLINE_ inline void merry_destroy_stack(MerryStack *stack)
{
    if (surelyF(stack == NULL))
        return;
    if (surelyT(stack->array != NULL))
        free(stack->array);
    free(stack);
}

_MERRY_INTERNAL_ mret_t merry_stack_resize(MerryStack *st)
{
    mqword_t new_len = st->size + st->add_per_resize;
    if (new_len > st->upper_lim)
        return RET_FAILURE; // cannot exceed upper limit
    st->array = (mqptr_t)realloc(st->array, 8 * (st->size + st->add_per_resize));
    if (st->array == NULL)
        return RET_FAILURE;
    st->size = new_len;
    return RET_SUCCESS;
}

mret_t merry_stack_push(MerryStack *st, mqword_t _to_push)
{
    if (stack_full(st))
    {
        if (st->dynamic != mtrue)
            return RET_FAILURE; // the stack is full
        // resize the stack
        if (merry_stack_resize(st) == RET_FAILURE)
            return RET_FAILURE;
    }
    // we have the stack and it should be resized if it was empty
    st->array[st->sp] = _to_push;
    st->sp++;
    return RET_SUCCESS;
}

_MERRY_ALWAYS_INLINE_ inline mret_t merry_stack_pop(MerryStack *st, mqptr_t store_in)
{
    if (stack_empty(st))
        return RET_FAILURE; // the stack is empty
    st->sp--;
    *store_in = st->array[st->sp];
    return RET_SUCCESS;
}

_MERRY_ALWAYS_INLINE_ inline void merry_stack_popn(MerryStack *st)
{
    // simply pop the top value and return
    // don't care about errors
    if (stack_empty(st))
        return;
    st->sp--;
}