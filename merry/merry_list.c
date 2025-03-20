#include "merry_list.h"

MerryList *merry_create_list(msize_t capacity, msize_t elem_len)
{
    MerryList *list = (MerryList *)malloc(sizeof(MerryList));
    if (!list)
        return RET_NULL;

    list->buf = malloc(sizeof(mptr_t) * elem_len + capacity);
    if (!list->buf)
    {
        free(list);
        return RET_NULL;
    }

    list->buf_cap = capacity;
    list->elem_len = elem_len;
    list->curr_ptr = (msize_t)(-1);
    list->max_ind = capacity - 1;

    return list;
}

mret_t merry_add_capacity_to_list(MerryList *list, msize_t _additional_cap)
{
    merry_check_ptr(list);
    merry_check_ptr(list->buf);

    // even if we fail, the buffer will still be valid

    // The main reason for all this bloated pointer checks and conditions is for the
    // VM to be secure. The cost is efficiency and speed.
    // These checks will be invalidated once we build Merry for release
    // before then these checks will ensure that Merry is safe from itself.
    // There could be cases where Merry, itself, produces garbage values
    // These "bloats" will help us detect those problems
    merry_assert(_additional_cap != 0);

    register mptr_t tmp = list->buf;
    register msize_t new_cap = list->buf_cap + _additional_cap;
    register mptr_t new_buf = malloc(sizeof(mptr_t) * list->elem_len + new_cap);

    if (!new_buf)
        return RET_FAILURE;

    memcpy(new_buf, list->buf, (list->curr_ptr + 1) * list->elem_len);
    list->buf = new_buf;
    list->buf_cap = new_cap;
    list->max_ind = new_cap - 1;

    free(tmp);
    return RET_SUCCESS;
}

mret_t merry_list_resize_list(MerryList *list, msize_t factor)
{
    merry_check_ptr(list);
    merry_check_ptr(list->buf);

    merry_assert(factor != 0);

    register mptr_t tmp = list->buf;
    register msize_t new_cap = list->buf_cap * factor;
    register mptr_t new_buf = malloc(sizeof(mptr_t) * list->elem_len + new_cap);

    if (!new_buf)
        return RET_FAILURE;

    memcpy(new_buf, list->buf, (list->curr_ptr + 1) * list->elem_len);
    list->buf = new_buf;
    list->buf_cap = new_cap;
    list->max_ind = new_cap - 1;

    free(tmp);
    return RET_SUCCESS;
}

mret_t merry_list_push(MerryList *list, mptr_t elem)
{
    merry_check_ptr(list);
    merry_check_ptr(elem);
    merry_check_ptr(list->buf);

    if (surelyF(merry_is_list_full(list)))
        return RET_FAILURE;

    list->curr_ptr++;
    register mptr_t curr_index = (mptr_t)((mstr_t)list->buf + (list->elem_len * list->curr_ptr));
    memcpy(curr_index, elem, list->elem_len);

    return RET_SUCCESS;
}

mptr_t merry_list_pop(MerryList *list)
{
    merry_check_ptr(list);
    merry_check_ptr(list->buf);

    if (surelyF(merry_is_list_empty(list)))
        return RET_NULL;

    register mptr_t res = (mptr_t)((mstr_t)list->buf + (list->elem_len * list->curr_ptr));
    list->curr_ptr--;
    return res;
}

void merry_erase_list(MerryList *list)
{
    merry_check_ptr(list);
    merry_check_ptr(list->buf);

    list->curr_ptr = (msize_t)(-1);
}

void merry_destroy_list(MerryList *list)
{
    merry_check_ptr(list);
    merry_check_ptr(list->buf);
    // we obviously don't care about the state of the pointers that were stored
    // if stored that is
    // This is a general purpose list to store anything
    free(list->buf);
    free(list);
}