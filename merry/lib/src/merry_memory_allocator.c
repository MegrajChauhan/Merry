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
    page->_remaining_page = _MERRY_ALLOC_PAGE_LEN_;
    page->_used_size = 0;
    return page; // we allocated a new page
}

static mret_t merry_allocator_misc_add_block(msize_t size)
{
    // size is aligned
    // we will first traverse the page list and see which page can fit the request
    MerryAllocPage *temp = allocator.pg_misc;
    msize_t _alloc_size = (size + _MERRY_ALLOC_BLOCK_SIZE_);
    while (temp != NULL)
    {
        // we have a certain requirement for this too
        // say we allocate size block and we have 2 bytes left over which is certainly not what we want
        // hence we either have to use those 2 bytes in the allocation as well or wait for an allocation that fits that size and get a new page
        // but that is not what we want right now
        // so we will check if we the remaining size is more than or at least equal to size + certain limit.
        if (temp->_remaining_page >= (_alloc_size))
        {
            MerryAllocBlock *new_block = (mbptr_t)temp->_start_address + temp->_used_size;
            temp->_used_size += _alloc_size;
            temp->_remaining_page -= _alloc_size;
            if (temp->_remaining_page >= (_MERRY_ALLOC_BLOCK_SIZE_ + 8))
            {
                new_block->_block_size = size;
            }else{
                // new_block->_block_size = 
            }
        }
    }
}

static void merry_allocator_update_misc(MerryAllocRetBlock details)
{
    // we update the allocated list
    // if the non free list is empty, add a new head
    if (details.page->entry_non_free == NULL)
    {
        // the head is empty
        details.block->next = details.block;
        details.block->prev = details.block;
        details.page->entry_non_free = details.block;
    }
    MerryAllocBlock *old_tail = details.page->entry_non_free->prev;
    old_tail->next->prev = details.block;
    old_tail->next = details.block;
    details.block->next = old_tail->next;
    details.block->prev = old_tail;
}

static MerryAllocRetBlock merry_allocator_find_free_misc(msize_t size)
{
    // size is aligned
    // we will need one function per page to traverse
    MerryAllocRetBlock ret_details = {.block = NULL, .page = NULL};
    MerryAllocPage *temp = allocator.pg_misc;
    // temp will certainly not be NULL
    while (temp != NULL)
    {
        // we will traverse all the free blocks in this page
        if (temp->entry_free != NULL)
        {
            // if the free list is not empty
            MerryAllocBlock *_temp = temp->entry_free;
            while (_temp != NULL)
            {
                // we won't split or merge for the sake of speed
                // This allocator will leave a large memory footprint but since the VM will be the only one using it
                // we can modify and upgrade this as we go
                if (_temp->_block_size == size)
                {
                    ret_details.block = _temp;
                    ret_details.page = temp;
                    return ret_details;
                }
                _temp = _temp->next; // go to the next block
            }
        }
        temp = temp->next_page; // go to the next page
    }
    return ret_details; // we found none that filled the description
}

static mptr_t merry_allocator_alloc_misc(msize_t size)
{
    // size is aligned
    // we will traverse the free list and see if we find any free blocks
    // if we have multiple misc pages, we will traverse each page and look for a free block
    // if we can't find one then we will allocate a new one
    MerryAllocRetBlock free_block = merry_allocator_find_free_misc(size);
    if (free_block.block != RET_NULL)
    {
        // we found a fitting, free block
        // we now just have to update the linked list
        if (free_block.block->next == free_block.block)
        {
            free_block.page->entry_free = NULL; // the free list is now empty
        }
        else
        {
            free_block.block->next->prev = free_block.block->prev;
            free_block.block->prev->next = free_block.block->next;
        }
        merry_allocator_update_misc(free_block);                               // update the linked list
        return (mptr_t)((mbptr_t)free_block.block + _MERRY_ALLOC_BLOCK_SIZE_); // return the allocated block
    }
    // if we are here then that means we found no free block
    // we have to allocate a new one then
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
    // first destroy the lock
    merry_alloc_destroy_lock();
    // then free all of the pages
    MerryAllocPage *page = allocator.pg_8; // starting from page 1
    MerryAllocPage *temp = page;
    while (page->next_page != NULL)
    {
        if (page->_is_mapped == mtrue)
        {
            // this page was mapped
            temp = page->next_page;
            _MERRY_ALLOC_UNMAP_PAGE_((mptr_t)page);
        }
        else
            temp = page->next_page;
        // we don't have to worry about pages that we got from extending the program's break since the Operating system will clean it up afterwards
        page = temp;
    }
    // we follow the same pattern for all the pages
    page = allocator.pg_16;
    while (page->next_page != NULL)
    {
        if (page->_is_mapped == mtrue)
        {
            temp = page->next_page;
            _MERRY_ALLOC_UNMAP_PAGE_((mptr_t)page);
        }
        else
            temp = page->next_page;
        page = temp;
    }
    page = allocator.pg_32;
    while (page->next_page != NULL)
    {
        if (page->_is_mapped == mtrue)
        {
            temp = page->next_page;
            _MERRY_ALLOC_UNMAP_PAGE_((mptr_t)page);
        }
        else
            temp = page->next_page;
        page = temp;
    }
    page = allocator.pg_64;
    while (page->next_page != NULL)
    {
        if (page->_is_mapped == mtrue)
        {
            temp = page->next_page;
            _MERRY_ALLOC_UNMAP_PAGE_((mptr_t)page);
        }
        else
            temp = page->next_page;
        page = temp;
    }
    page = allocator.pg_misc;
    while (page->next_page != NULL)
    {
        if (page->_is_mapped == mtrue)
        {
            temp = page->next_page;
            _MERRY_ALLOC_UNMAP_PAGE_((mptr_t)page);
        }
        else
            temp = page->next_page;
        page = temp;
    }
    // we have now freed all the pages
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
        // we will simply allocate from the misc page
    }
end:
    merry_mutex_unlock(&allocator.lock);
    return new_block;
}