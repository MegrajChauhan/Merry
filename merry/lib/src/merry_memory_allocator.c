#include "../include/merry_memory_allocator.h"

// specifically init the lock for the allocator
_MERRY_INTERNAL_ mret_t merry_alloc_init_lock()
{
#if defined(_MERRY_THREADS_POSIX_)
    if (pthread_mutex_init(&allocator.lock.mutex, NULL) != 0)
    {
        return RET_FAILURE;
    }
#endif
    return RET_SUCCESS;
}

_MERRY_INTERNAL_ void merry_alloc_destroy_lock()
{
#if defined(_MERRY_THREADS_POSIX_)
    pthread_mutex_destroy(&allocator.lock.mutex);
#endif
}

_MERRY_INTERNAL_ MerryAllocPage *merry_allocator_get_page()
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

_MERRY_INTERNAL_ void merry_allocator_update_misc(MerryAllocBlock *details)
{
    // we update the allocated list
    // if the non free list is empty, add a new head
    if (details->parent_page->entry_non_free == NULL)
    {
        // the head is empty
        details->next = details;
        details->prev = details;
        details->parent_page->entry_non_free = details;
    }
    MerryAllocBlock *old_tail = details->parent_page->entry_non_free->prev;
    old_tail->next->prev = details;
    old_tail->next = details;
    details->next = old_tail->next;
    details->prev = old_tail;
}

_MERRY_INTERNAL_ mptr_t merry_allocator_misc_add_block(msize_t size)
{
    // size is aligned
    // we will first traverse the page list and see which page can fit the request
    MerryAllocPage *temp = allocator.pg_misc;
    msize_t _alloc_size = (size + _MERRY_ALLOC_BLOCK_SIZE_);
    MerryAllocBlock *new_block;
    while (temp != NULL)
    {
        // we have a certain requirement for this too
        // say we allocate size block and we have 2 bytes left over which is certainly not what we want
        // hence we either have to use those 2 bytes in the allocation as well or wait for an allocation that fits that size and get a new page
        // but that is not what we want right now
        // so we will check if we the remaining size is more than or at least equal to size + certain limit.
        if (temp->_remaining_page >= (_alloc_size))
        {
            new_block = (MerryAllocBlock *)((mbptr_t)temp->_start_address + temp->_used_size);
            temp->_used_size += _alloc_size;
            temp->_remaining_page -= _alloc_size;
            if (temp->_remaining_page >= (_MERRY_ALLOC_BLOCK_SIZE_ + 8))
            {
                new_block->_block_size = size;
            }
            else
            {
                // after allocation we won't have enough memory to allocate to any other block
                // so we just allocate that remaining bytes to the requested block as well
                new_block->_block_size = size + temp->_remaining_page;
                temp->_used_size += temp->_remaining_page;
                temp->_remaining_page = 0; // we have no more memory left now
            }
            // we now have to setup new_block
            // this becomes the new tail or becomes the new head
            new_block->parent_page = temp;
            merry_allocator_update_misc(new_block); // update
            return (mbptr_t)new_block + _MERRY_ALLOC_BLOCK_SIZE_;
        }
        // go to the next page
        temp = temp->next_page;
    }
    // since we are here than it would mean that we don't have any page that can fulfill the request
    // we need to allocate a new page
    MerryAllocPage *new_page = merry_allocator_get_page();
    if (new_page == NULL)
    {
        // we cannot to anything anymore
        return RET_NULL;
    }
    // we now can do something
    // first we make this new page the head
    new_page->next_page = allocator.pg_misc;
    allocator.pg_misc = new_page;
    // we now allocate a new block
    // since this is a new page it is empty
    new_block = (MerryAllocBlock *)new_page->_start_address;
    new_block->parent_page = new_page;
    new_block->_block_size = size;
    new_page->_remaining_page -= _alloc_size;
    new_page->_used_size += _alloc_size;
    merry_allocator_update_misc(new_block); // update
    return (mbptr_t)new_block + _MERRY_ALLOC_BLOCK_SIZE_;
}

_MERRY_INTERNAL_ MerryAllocBlock *merry_allocator_find_free_misc(msize_t size)
{
    // size is aligned
    // we will need one function per page to traverse
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
                    return _temp;
                }
                _temp = _temp->next; // go to the next block
            }
        }
        temp = temp->next_page; // go to the next page
    }
    return RET_NULL; // we found none that filled the description
}

_MERRY_INTERNAL_ mptr_t merry_allocator_alloc_misc(msize_t size)
{
    // size is aligned
    // we will traverse the free list and see if we find any free blocks
    // if we have multiple misc pages, we will traverse each page and look for a free block
    // if we can't find one then we will allocate a new one
    MerryAllocBlock *free_block = merry_allocator_find_free_misc(size);
    if (free_block != RET_NULL)
    {
        // we found a fitting, free block
        // we now just have to update the linked list
        if (free_block->next == free_block)
        {
            free_block->parent_page->entry_free = NULL; // the free list is now empty
        }
        else
        {
            free_block->next->prev = free_block->prev;
            free_block->prev->next = free_block->next;
        }
        merry_allocator_update_misc(free_block);                         // update the linked list
        return (mptr_t)((mbptr_t)free_block + _MERRY_ALLOC_BLOCK_SIZE_); // return the allocated block
    }
    // if we are here then that means we found no free block
    // we have to allocate a new one then
    mptr_t temp = merry_allocator_misc_add_block(size); // try getting new block
    return temp == RET_NULL ? RET_NULL : temp;
}

///////////////////////////////////////////////////////////////////////////////////////
//                                                                                   //
//                                                                                   //
//                               REDUNDANCY ALERT!                                   //
//                                                                                   //
//                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////

_MERRY_INTERNAL_ void merry_allocator_update_pg8(MerryAllocBlock *details)
{
    if (details->parent_page->entry_non_free == NULL)
    {
        // the head is empty
        details->next = details;
        details->prev = details;
        details->parent_page->entry_non_free = details;
    }
    MerryAllocBlock *old_tail = details->parent_page->entry_non_free->prev;
    old_tail->next->prev = details;
    old_tail->next = details;
    details->next = old_tail->next;
    details->prev = old_tail;
}

_MERRY_INTERNAL_ mptr_t merry_allocator_pg8_add_block()
{
    // size is aligned
    MerryAllocPage *temp = allocator.pg_8;
    msize_t _alloc_size = (8 + _MERRY_ALLOC_BLOCK_SIZE_);
    MerryAllocBlock *new_block;
    while (temp != NULL)
    {
        if (temp->_remaining_page >= (_alloc_size))
        {
            new_block = (MerryAllocBlock *)((mbptr_t)temp->_start_address + temp->_used_size);
            temp->_used_size += _alloc_size;
            temp->_remaining_page -= _alloc_size;
            if (temp->_remaining_page >= (_MERRY_ALLOC_BLOCK_SIZE_ + 8))
            {
                new_block->_block_size = 8;
            }
            else
            {
                new_block->_block_size = 8 + temp->_remaining_page;
                temp->_used_size += temp->_remaining_page;
                temp->_remaining_page = 0;
            }
            new_block->parent_page = temp;
            merry_allocator_update_pg8(new_block); // update
            return (mbptr_t)new_block + _MERRY_ALLOC_BLOCK_SIZE_;
        }
        temp = temp->next_page;
    }
    MerryAllocPage *new_page = merry_allocator_get_page();
    if (new_page == NULL)
    {
        return RET_NULL;
    }
    new_page->next_page = allocator.pg_misc;
    allocator.pg_misc = new_page;
    new_block = (MerryAllocBlock *)new_page->_start_address;
    new_block->parent_page = new_page;
    new_block->_block_size = 8;
    new_page->_remaining_page -= _alloc_size;
    new_page->_used_size += _alloc_size;
    merry_allocator_update_pg8(new_block); // update
    return (mbptr_t)new_block + _MERRY_ALLOC_BLOCK_SIZE_;
}

_MERRY_INTERNAL_ MerryAllocBlock *merry_allocator_find_free_pg8()
{
    MerryAllocPage *temp = allocator.pg_8;
    while (temp != NULL)
    {
        if (temp->entry_free != NULL)
        {
            MerryAllocBlock *_temp = temp->entry_free;
            while (_temp != NULL)
            {
                if (_temp->_block_size == 8)
                {
                    return _temp;
                }
                _temp = _temp->next; // go to the next block
            }
        }
        temp = temp->next_page; // go to the next page
    }
    return RET_NULL; // we found none that filled the description
}

_MERRY_INTERNAL_ mptr_t merry_allocator_alloc_pg8()
{
    MerryAllocBlock *free_block = merry_allocator_find_free_pg8();
    if (free_block != RET_NULL)
    {
        if (free_block->next == free_block)
        {
            free_block->parent_page->entry_free = NULL; // the free list is now empty
        }
        else
        {
            free_block->next->prev = free_block->prev;
            free_block->prev->next = free_block->next;
        }
        merry_allocator_update_pg8(free_block);                          // update the linked list
        return (mptr_t)((mbptr_t)free_block + _MERRY_ALLOC_BLOCK_SIZE_); // return the allocated block
    }
    mptr_t temp = merry_allocator_pg8_add_block(); // try getting new block
    return temp == RET_NULL ? RET_NULL : temp;
}

_MERRY_INTERNAL_ void merry_allocator_update_pg16(MerryAllocBlock *details)
{
    if (details->parent_page->entry_non_free == NULL)
    {
        // the head is empty
        details->next = details;
        details->prev = details;
        details->parent_page->entry_non_free = details;
    }
    MerryAllocBlock *old_tail = details->parent_page->entry_non_free->prev;
    old_tail->next->prev = details;
    old_tail->next = details;
    details->next = old_tail->next;
    details->prev = old_tail;
}

_MERRY_INTERNAL_ mptr_t merry_allocator_pg16_add_block()
{
    // size is aligned
    MerryAllocPage *temp = allocator.pg_16;
    msize_t _alloc_size = (16 + _MERRY_ALLOC_BLOCK_SIZE_);
    MerryAllocBlock *new_block;
    while (temp != NULL)
    {
        if (temp->_remaining_page >= (_alloc_size))
        {
            new_block = (MerryAllocBlock *)((mbptr_t)temp->_start_address + temp->_used_size);
            temp->_used_size += _alloc_size;
            temp->_remaining_page -= _alloc_size;
            if (temp->_remaining_page >= (_MERRY_ALLOC_BLOCK_SIZE_ + 16))
            {
                new_block->_block_size = 16;
            }
            else
            {
                new_block->_block_size = 16 + temp->_remaining_page;
                temp->_used_size += temp->_remaining_page;
                temp->_remaining_page = 0;
            }
            new_block->parent_page = temp;
            merry_allocator_update_pg16(new_block); // update
            return (mbptr_t)new_block + _MERRY_ALLOC_BLOCK_SIZE_;
        }
        temp = temp->next_page;
    }
    MerryAllocPage *new_page = merry_allocator_get_page();
    if (new_page == NULL)
    {
        return RET_NULL;
    }
    new_page->next_page = allocator.pg_misc;
    allocator.pg_misc = new_page;
    new_block = (MerryAllocBlock *)new_page->_start_address;
    new_block->parent_page = new_page;
    new_block->_block_size = 16;
    new_page->_remaining_page -= _alloc_size;
    new_page->_used_size += _alloc_size;
    merry_allocator_update_pg16(new_block); // update
    return (mbptr_t)new_block + _MERRY_ALLOC_BLOCK_SIZE_;
}

_MERRY_INTERNAL_ MerryAllocBlock *merry_allocator_find_free_pg16()
{
    MerryAllocPage *temp = allocator.pg_16;
    while (temp != NULL)
    {
        if (temp->entry_free != NULL)
        {
            MerryAllocBlock *_temp = temp->entry_free;
            while (_temp != NULL)
            {
                if (_temp->_block_size == 16)
                {
                    return _temp;
                }
                _temp = _temp->next; // go to the next block
            }
        }
        temp = temp->next_page; // go to the next page
    }
    return RET_NULL; // we found none that filled the description
}

_MERRY_INTERNAL_ mptr_t merry_allocator_alloc_pg16()
{
    MerryAllocBlock *free_block = merry_allocator_find_free_pg16();
    if (free_block != RET_NULL)
    {
        if (free_block->next == free_block)
        {
            free_block->parent_page->entry_free = NULL; // the free list is now empty
        }
        else
        {
            free_block->next->prev = free_block->prev;
            free_block->prev->next = free_block->next;
        }
        merry_allocator_update_pg16(free_block);                         // update the linked list
        return (mptr_t)((mbptr_t)free_block + _MERRY_ALLOC_BLOCK_SIZE_); // return the allocated block
    }
    mptr_t temp = merry_allocator_pg16_add_block(); // try getting new block
    return temp == RET_NULL ? RET_NULL : temp;
}

_MERRY_INTERNAL_ void merry_allocator_update_pg32(MerryAllocBlock *details)
{
    if (details->parent_page->entry_non_free == NULL)
    {
        details->next = details;
        details->prev = details;
        details->parent_page->entry_non_free = details;
    }
    MerryAllocBlock *old_tail = details->parent_page->entry_non_free->prev;
    old_tail->next->prev = details;
    old_tail->next = details;
    details->next = old_tail->next;
    details->prev = old_tail;
}

_MERRY_INTERNAL_ mptr_t merry_allocator_pg32_add_block()
{
    // size is aligned
    MerryAllocPage *temp = allocator.pg_32;
    msize_t _alloc_size = (32 + _MERRY_ALLOC_BLOCK_SIZE_);
    MerryAllocBlock *new_block;
    while (temp != NULL)
    {
        if (temp->_remaining_page >= (_alloc_size))
        {
            new_block = (MerryAllocBlock *)((mbptr_t)temp->_start_address + temp->_used_size);
            temp->_used_size += _alloc_size;
            temp->_remaining_page -= _alloc_size;
            if (temp->_remaining_page >= (_MERRY_ALLOC_BLOCK_SIZE_ + 32))
            {
                new_block->_block_size = 32;
            }
            else
            {
                new_block->_block_size = 32 + temp->_remaining_page;
                temp->_used_size += temp->_remaining_page;
                temp->_remaining_page = 0;
            }
            new_block->parent_page = temp;
            merry_allocator_update_pg32(new_block); // update
            return (mbptr_t)new_block + _MERRY_ALLOC_BLOCK_SIZE_;
        }
        temp = temp->next_page;
    }
    MerryAllocPage *new_page = merry_allocator_get_page();
    if (new_page == NULL)
    {
        return RET_NULL;
    }
    new_page->next_page = allocator.pg_misc;
    allocator.pg_misc = new_page;
    new_block = (MerryAllocBlock *)new_page->_start_address;
    new_block->parent_page = new_page;
    new_block->_block_size = 32;
    new_page->_remaining_page -= _alloc_size;
    new_page->_used_size += _alloc_size;
    merry_allocator_update_pg32(new_block); // update
    return (mbptr_t)new_block + _MERRY_ALLOC_BLOCK_SIZE_;
}

_MERRY_INTERNAL_ MerryAllocBlock *merry_allocator_find_free_pg32()
{
    MerryAllocPage *temp = allocator.pg_32;
    while (temp != NULL)
    {
        if (temp->entry_free != NULL)
        {
            MerryAllocBlock *_temp = temp->entry_free;
            while (_temp != NULL)
            {
                if (_temp->_block_size == 32)
                {
                    return _temp;
                }
                _temp = _temp->next; // go to the next block
            }
        }
        temp = temp->next_page; // go to the next page
    }
    return RET_NULL; // we found none that filled the description
}

_MERRY_INTERNAL_ mptr_t merry_allocator_alloc_pg32()
{
    MerryAllocBlock *free_block = merry_allocator_find_free_pg32();
    if (free_block != RET_NULL)
    {
        if (free_block->next == free_block)
        {
            free_block->parent_page->entry_free = NULL; // the free list is now empty
        }
        else
        {
            free_block->next->prev = free_block->prev;
            free_block->prev->next = free_block->next;
        }
        merry_allocator_update_pg32(free_block);                         // update the linked list
        return (mptr_t)((mbptr_t)free_block + _MERRY_ALLOC_BLOCK_SIZE_); // return the allocated block
    }
    mptr_t temp = merry_allocator_pg32_add_block(); // try getting new block
    return temp == RET_NULL ? RET_NULL : temp;
}

_MERRY_INTERNAL_ void merry_allocator_update_pg64(MerryAllocBlock *details)
{
    if (details->parent_page->entry_non_free == NULL)
    {
        details->next = details;
        details->prev = details;
        details->parent_page->entry_non_free = details;
    }
    MerryAllocBlock *old_tail = details->parent_page->entry_non_free->prev;
    old_tail->next->prev = details;
    old_tail->next = details;
    details->next = old_tail->next;
    details->prev = old_tail;
}

_MERRY_INTERNAL_ mptr_t merry_allocator_pg64_add_block()
{
    MerryAllocPage *temp = allocator.pg_64;
    msize_t _alloc_size = (64 + _MERRY_ALLOC_BLOCK_SIZE_);
    MerryAllocBlock *new_block;
    while (temp != NULL)
    {
        if (temp->_remaining_page >= (_alloc_size))
        {
            new_block = (MerryAllocBlock *)((mbptr_t)temp->_start_address + temp->_used_size);
            temp->_used_size += _alloc_size;
            temp->_remaining_page -= _alloc_size;
            if (temp->_remaining_page >= (_MERRY_ALLOC_BLOCK_SIZE_ + 64))
            {
                new_block->_block_size = 64;
            }
            else
            {
                new_block->_block_size = 64 + temp->_remaining_page;
                temp->_used_size += temp->_remaining_page;
                temp->_remaining_page = 0;
            }
            new_block->parent_page = temp;
            merry_allocator_update_pg64(new_block); // update
            return (mbptr_t)new_block + _MERRY_ALLOC_BLOCK_SIZE_;
        }
        temp = temp->next_page;
    }
    MerryAllocPage *new_page = merry_allocator_get_page();
    if (new_page == NULL)
    {
        return RET_NULL;
    }
    new_page->next_page = allocator.pg_misc;
    allocator.pg_misc = new_page;
    new_block = (MerryAllocBlock *)new_page->_start_address;
    new_block->parent_page = new_page;
    new_block->_block_size = 64;
    new_page->_remaining_page -= _alloc_size;
    new_page->_used_size += _alloc_size;
    merry_allocator_update_pg64(new_block); // update
    return (mbptr_t)new_block + _MERRY_ALLOC_BLOCK_SIZE_;
}

_MERRY_INTERNAL_ MerryAllocBlock *merry_allocator_find_free_pg64()
{
    MerryAllocPage *temp = allocator.pg_64;
    while (temp != NULL)
    {
        if (temp->entry_free != NULL)
        {
            // if the free list is not empty
            MerryAllocBlock *_temp = temp->entry_free;
            while (_temp != NULL)
            {
                if (_temp->_block_size == 64)
                {
                    return _temp;
                }
                _temp = _temp->next; // go to the next block
            }
        }
        temp = temp->next_page; // go to the next page
    }
    return RET_NULL; // we found none that filled the description
}

_MERRY_INTERNAL_ mptr_t merry_allocator_alloc_pg64()
{
    MerryAllocBlock *free_block = merry_allocator_find_free_pg64();
    if (free_block != RET_NULL)
    {
        if (free_block->next == free_block)
        {
            free_block->parent_page->entry_free = NULL; // the free list is now empty
        }
        else
        {
            free_block->next->prev = free_block->prev;
            free_block->prev->next = free_block->next;
        }
        merry_allocator_update_pg64(free_block);                         // update the linked list
        return (mptr_t)((mbptr_t)free_block + _MERRY_ALLOC_BLOCK_SIZE_); // return the allocated block
    }
    mptr_t temp = merry_allocator_pg64_add_block(); // try getting new block
    return temp == RET_NULL ? RET_NULL : temp;
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
        return RET_FAILURE; // we failed
    if ((allocator.pg_16 = merry_allocator_get_page()) == RET_NULL)
        return RET_FAILURE; // we failed
    if ((allocator.pg_32 = merry_allocator_get_page()) == RET_NULL)
        return RET_FAILURE; // we failed
    if ((allocator.pg_64 = merry_allocator_get_page()) == RET_NULL)
        return RET_FAILURE; // we failed
    if ((allocator.pg_misc = merry_allocator_get_page()) == RET_NULL)
        return RET_FAILURE; // we failed
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
    switch (size)
    {
    case 8:
        new_block = merry_allocator_alloc_pg8();
        break;
    case 16:
        new_block = merry_allocator_alloc_pg16();
        break;
    case 24: // even if the request is of only 24 bytes, we are willing to sacrifice 8 extra bytes
    case 32:
        new_block = merry_allocator_alloc_pg32();
        break;
    case 56:
    case 64: // here we are only willing to sacrifice 8 bytes
        new_block = merry_allocator_alloc_pg64();
        break;
    default:
        new_block = merry_allocator_alloc_misc(size);
    }
    goto end;
end:
    merry_mutex_unlock(&allocator.lock);
    return new_block;
}

_MERRY_INTERNAL_ void merry_alloc_free_memory(MerryAllocBlock *block)
{
    // to free allocated memory
    // the block itself holds the information about the parent page
    MerryAllocPage *parent_page = block->parent_page;
    // we just have to update the parent page's free list
    if (parent_page->entry_non_free == block)
    {
        // if block is the head
        if (block->next == block)
        {
            // if there was only one allocated block
            parent_page->entry_non_free = NULL; // no more allocated blocks
        }
        else
        {
            block->next->prev = block->prev;
            block->prev->next = block->next;
            parent_page->entry_non_free = block->next; // update the head
        }
    }
    else
    {
        // the block is not the head
        block->next->prev = block->prev;
        block->prev->next = block->next;
    }
    // now update the free list
    if (parent_page->entry_free == NULL)
    {
        // if the page has no free block
        block->next = block;
        block->prev = block;
        parent_page->entry_free = block;
    }
    else
    {
        MerryAllocBlock *old_tail = parent_page->entry_free->prev;
        old_tail->next->prev = block;
        old_tail->next = block;
        block->next = parent_page->entry_free;
        block->prev = old_tail;
    }
    // that is all
}

void merry_free(mptr_t _ptr)
{
    if (surelyF(_ptr == NULL))
        return;
    MerryAllocBlock *block = (MerryAllocBlock *)((mbptr_t)_ptr - _MERRY_ALLOC_BLOCK_SIZE_);
    merry_alloc_free_memory(block);
}

mptr_t merry_realloc(mptr_t _ptr, msize_t _new_size)
{
    // reallocation is relatively harder to do
    // While the allocator is not designed to do large and tough allocations and tasks, we have to make it do reallocations
    // For this the simplest method i can think of is:
    // Find a new memory block fitting the description.
    // Copy the contents from old memory.
    // Free the old memory.
    // Since the allocator cannot handle requests for more than a few kilobytes, this should be the ideal choice as copying this much memory shouldn't be a problem.
    if (surelyF(_ptr == NULL))
        return RET_NULL;
    // get the block
    MerryAllocBlock *block = (MerryAllocBlock *)((mbptr_t)_ptr - _MERRY_ALLOC_BLOCK_SIZE_);
    // now we need to allocate a new block
    mptr_t new_block = merry_malloc(_new_size);
    if (new_block == NULL)
        return RET_NULL; // we failed
                         // if we didn't fail, copy the old contents and free the block
    if (memcpy(new_block, (const mptr_t)_ptr, block->_block_size) != new_block)
    {
        return RET_NULL; // we failed again
    }
    // we succeeded!
    // new_block should contain the data
    merry_alloc_free_memory(block); // directly free the memory
    return new_block;               // YAY!
}

mptr_t merry_lalloc(msize_t size)
{
    // we simply get a memory page of size bytes and then be done with it
    // we can do alignment checks and align the pages that we ask from the OS for faster accesses but we are dumb so we won't
    mptr_t lalloced = _MERRY_LALLOC_MAP_PAGE_(size); // we don't align size since this is a temporary mapping and doesn't need alignment
    return lalloced == _MERRY_RET_GET_ERROR_ ? RET_NULL : lalloced;
}

void merry_lfree(mptr_t page, msize_t size)
{
    // we are assuming that page is a valid pointer and unmap will not throw any error
    _MERRY_LALLOC_UNMAP_PAGE_(page, size);
}