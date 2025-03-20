#include "merry_memory.h"

MerryNormalMemoryPage *merry_create_normal_memory_page()
{
    MerryNormalMemoryPage *pg = (MerryNormalMemoryPage *)malloc(sizeof(MerryNormalMemoryPage));
    if (!pg)
        return RET_NULL;

    pg->buf = NULL;
    pg->init = mfalse;

    return pg;
}

mret_t merry_initialize_normal_memory_page(MerryNormalMemoryPage *pg)
{
    merry_check_ptr(pg);

    // reinitialization won't work
    merry_assert(pg->buf == NULL);

    if ((pg->buf = merry_get_memory_page(_MERRY_PAGE_LEN_, _MERRY_PROT_DEFAULT_, _MERRY_FLAG_DEFAULT_)) == _MERRY_FAILED_TO_GET_PAGE_)
        return RET_FAILURE;

    pg->init = mtrue;
    return RET_SUCCESS;
}

MerryNormalMemoryPage *merry_obtain_normal_memory_page()
{
    MerryNormalMemoryPage *pg = merry_create_normal_memory_page();
    if (!pg)
        return RET_FAILURE;
    if (merry_initialize_normal_memory_page(pg) == RET_FAILURE)
    {
        free(pg);
        return RET_NULL;
    }

    return pg;
}

void merry_return_normal_memory_page(MerryNormalMemoryPage *pg)
{
    merry_check_ptr(pg);
    if (surelyT(pg->buf))
    {
        merry_return_memory_page(pg->buf, _MERRY_PAGE_LEN_);
    }
    free(pg);
}