#include "merry_dynamic_list.h"

// It is all basically the same as MerryList
mret_t merry_dynamic_list_push(MerryDynamicList *list, mptr_t elem)
{
    merry_check_ptr(list);
    merry_check_ptr(elem);
    merry_check_ptr(list->buf);

    if (surelyF(merry_is_dyn_list_full(list)))
    {
        // This is where the behavior shifts compared to the MerryList

        // try resizing
        if (merry_dynamic_list_resize(list, 2) == RET_FAILURE)
            return RET_FAILURE; // 2 is an arbitary internal constant
    }

    list->curr_ptr++;
    register mptr_t curr_index = (mptr_t)((mstr_t)list->buf + (list->elem_len * list->curr_ptr));
    memcpy(curr_index, elem, list->elem_len);

    return RET_SUCCESS;
}
