#include "internals/merry_page_allocator.h"

mptr_t merry_pgalloc_get_page(void)
{
    mptr_t page = _MERRY_PGALLOC_GET_PAGE_;
    if (page == _MERRY_RET_GET_ERROR_)
    {
        // error
        return RET_NULL; // we failed to get a new page
    }
    return page; // return the newly allocated page
}

mret_t merry_pgalloc_give_page(mptr_t address)
{
    if (surelyF(address == (mptr_t)-1))
        return RET_FAILURE; // highly unlikely
    if (_MERRY_PGALLOC_GIVE_PAGE_(address) == _MERRY_RET_GIVE_ERROR_)
        return RET_FAILURE;
    return RET_SUCCESS;
}