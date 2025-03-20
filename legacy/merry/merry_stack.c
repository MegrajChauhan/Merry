#include "merry_stack.h"

MerryStack *merry_init_stack(msize_t len, mbool_t dynamic, msize_t upper_lim, msize_t per_resize)
{
    inlog("Creating a STACK");
    MerryStack *st = (MerryStack *)malloc(sizeof(MerryStack));
    if (st == NULL)
    {
        mreport("Failed to allocate a STACK");
        return RET_NULL;
    }
    st->array = (mqptr_t)malloc(8 * len);
    if (st->array == NULL)
    {
        free(st);
        mreport("Failed to allocate a STACK[ARRAY]");
        return RET_NULL;
    }
    st->size = len;
    st->sp = (mqword_t)(-1);
    st->dynamic = dynamic;
    st->add_per_resize = per_resize;
    st->upper_lim = (upper_lim == 0) ? 1000 : upper_lim; // upper limit == 0 means no limit but since we want it to be limited, 1000 is the limit
    return st;
}

_MERRY_ALWAYS_INLINE_ inline void merry_destroy_stack(MerryStack *stack)
{
    massert_field(stack, array);
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
    {
        mreport("Failed to resize a STACK");
        return RET_FAILURE;
    }
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
    st->sp++;
    st->array[st->sp] = _to_push;
    return RET_SUCCESS;
}

_MERRY_ALWAYS_INLINE_ inline mret_t merry_stack_pop(MerryStack *st, mqptr_t store_in)
{
    if (stack_empty(st))
        return RET_FAILURE; // the stack is empty
    *store_in = st->array[st->sp];
    st->sp--;
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