#include "../include/merry_memory_allocator.h"

// specifically init the lock for the allocator
static mret_t merry_alloc_init_lock()
{
#if defined(_MERRY_THREADS_POSIX_)
    if (pthread_mutex_init(&allocator.lock.mutex, NULL) != 0)
    {
        return RET_FAILURE;
    }
#endif
    return RET_SUCCESS;
}

static void merry_alloc_destroy_lock()
{
#if defined(_MERRY_THREADS_POSIX_)
    pthread_mutex_destroy(&allocator.lock.mutex);
#endif
}

static MerryAllocPage *merry_allocator_get_page()
{
    // try to get a alloc page worth of memory from the OS and return it
    mptr_t _new_page;
    mbool_t _is_mapped = mfalse;
#if defined(_MERRY_MEM_BRK_SUPPORT_)
    // we have support for memory break support
    // we will first try to make this request
    // we also need memory for the page's metadata itself
    _new_page = _MERRY_MEM_GET_CURRENT_BRK_POINT_; // get the current break point
    if ((_MERRY_MEM_INCREASE_BRK_POINT_(_MERRY_ALLOC_PAGE_LEN_ + _MERRY_ALLOCPG_SIZE_)) == _MERRY_MEM_BRK_ERROR_)
    {
        // we failed in this case
        // we simply cannot return, instead we will try mapping as well
        _new_page = NULL;
    }
#endif
    // check if we actually allocated a page
    MerryAllocPage *page;
    if (_new_page != NULL)
    {
        page = (MerryAllocPage *)_new_page;
        page->_is_mapped = mfalse;
    }
    else
    {
        if ((_new_page = _MERRY_ALLOC_MAP_PAGE_) == _MERRY_RET_GET_ERROR_)
        {
            // we failed
            return RET_NULL;
        }
        page = (MerryAllocPage *)_new_page;
        page->_is_mapped = mtrue;
    }
    page->entry_free = NULL;
    page->entry_non_free = NULL;
    page->next_page = NULL;
    page->_start_address = (mbptr_t)_new_page + _MERRY_ALLOCPG_SIZE_;
    return page; // we allocated a new page
}

mret_t merry_allocator_init()
{
    // This function will be called only once
    // initialize the lock first
    if (merry_alloc_init_lock() == RET_FAILURE)
        return RET_FAILURE;
    // we now have to allocate all of the pages
    // firstly the 8 bytes
    if ((allocator.pg_8 = merry_allocator_get_page()) == RET_NULL)
        return NULL; // we failed
    if ((allocator.pg_16 = merry_allocator_get_page()) == RET_NULL)
        return NULL; // we failed
    if ((allocator.pg_32 = merry_allocator_get_page()) == RET_NULL)
        return NULL; // we failed
    if ((allocator.pg_64 = merry_allocator_get_page()) == RET_NULL)
        return NULL; // we failed
    if ((allocator.pg_misc = merry_allocator_get_page()) == RET_NULL)
        return NULL; // we failed
    // we currently have no free or allocated blocks
    return RET_SUCCESS; // we made it this far meaning we succeeded
}

void merry_allocator_destroy()
{
    
}

mptr_t merry_malloc(msize_t size)
{
    // the allocator will have been allocated by now
    merry_mutex_lock(&allocator.lock);
    mptr_t new_block = NULL;
    size = merry_align_size(size);
    if (surelyF(size == 0))
        goto end;
    // we will check if size falls under the misc catagory
    // say the request is for 48 bytes which is the closest to 64 bytes but then there will be a loss of a lot of valuable bytes
    // in this case that doesn't matter as this is for the VM's internal only
    if (size > 64)
    {
        // that straight falls into misc category
    }
end:
    merry_mutex_unlock(&allocator.lock);
    return new_block;
}