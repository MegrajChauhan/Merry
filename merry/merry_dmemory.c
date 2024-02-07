#include "internals/merry_dmemory.h"

_MERRY_INTERNAL_ MerryDMemPage *merry_mem_allocate_new_mempage()
{
    MerryDMemPage *new_page = (MerryDMemPage *)malloc(sizeof(MerryDMemPage));
    if (new_page == RET_NULL)
    {
        // failed allocation
        return RET_NULL;
    }
    // try allocating the address space
    if ((new_page->address_space = (mbptr_t)_MERRY_DMEMORY_PGALLOC_MAP_PAGE_) == NULL)
    {
        free(new_page);
        return RET_NULL; // we failed
    }
    // initialize the page's lock
    if ((new_page->lock = merry_mutex_init()) == RET_NULL)
    {
        _MERRY_DMEMORY_PGALLOC_UNMAP_PAGE_(new_page->address_space);
        free(new_page);
        return RET_NULL;
    }
    // everything went successfully
    return new_page;
}

_MERRY_INTERNAL_ MerryDMemPage *merry_mem_allocate_new_mempage_provided(mqptr_t page)
{
    MerryDMemPage *new_page = (MerryDMemPage *)malloc(sizeof(MerryDMemPage));
    if (new_page == RET_NULL)
    {
        // failed allocation
        return RET_NULL;
    }
    // try allocating the address space
    new_page->address_space = (mbptr_t)page; // we were provided
    // initialize the page's lock
    if ((new_page->lock = merry_mutex_init()) == RET_NULL)
    {
        _MERRY_DMEMORY_PGALLOC_UNMAP_PAGE_(new_page->address_space);
        free(new_page);
        return RET_NULL;
    }
    // everything went successfully
    _log_(_MEM_, "Page Allocation", "Allocating memory provided");
    return new_page;
}

// helper function :free an allocate memory page
_MERRY_INTERNAL_ void merry_mem_free_mempage(MerryDMemPage *page)
{
    if (surelyF(page == NULL))
        return;
    if (page->lock != NULL)
    {
        merry_mutex_destroy(page->lock);
    }
    if (surelyT(page->address_space != NULL))
    {
        _MERRY_DMEMORY_PGALLOC_UNMAP_PAGE_(page->address_space);
    }
    free(page); // that is all
}

// exposed function: initialize memory with num_of_pages pages
MerryDMemory *merry_dmemory_init(msize_t num_of_pages)
{
    _llog_(_MEM_, "INIT", "Intializing memory with %lu pages", num_of_pages);
    MerryDMemory *memory = (MerryDMemory *)malloc(sizeof(MerryDMemory));
    if (memory == RET_NULL)
    {
        // we failed
        _log_(_MEM_, "FAILED", "Memory inti failed");
        return RET_NULL;
    }
    memory->error = MERRY_ERROR_NONE;
    memory->number_of_pages = 0;
    memory->pages = (MerryDMemPage **)malloc(sizeof(MerryDMemPage *) * num_of_pages);
    if (memory->pages == RET_NULL)
    {
        // failed
        _log_(_MEM_, "FAILED", "Mmeory init failed");
        free(memory);
        return RET_NULL;
    }
    // now we need to initialize every single page
    for (msize_t i = 0; i < num_of_pages; i++, memory->number_of_pages++)
    {
        _llog_(_MEM_, "ALLOCATING PAGES", "Allocating page %lu", i);
        memory->pages[i] = merry_mem_allocate_new_mempage();
        if (memory->pages[i] == RET_NULL)
        {
            // failure
            _log_(_MEM_, "FAILED", "Memory intialization failed while allocating pages");
            merry_dmemory_free(memory);
            return RET_NULL;
        }
    }
    // we have allocated everything successfully
    _log_(_MEM_, "MEM_INIT_SUCCESS", "Memory successfully initialized");
    return memory;
}

MerryDMemory *merry_dmemory_init_provided(mqptr_t *mapped_pages, msize_t num_of_pages)
{
    // just perform the regular allocation but don't map new pages
    // instead use the already mapped ones
    _llog_(_MEM_, "INIT", "Intializing memory with %lu pages", num_of_pages);
    MerryDMemory *memory = (MerryDMemory *)malloc(sizeof(MerryDMemory));
    if (memory == RET_NULL)
    {
        // we failed
        _log_(_MEM_, "FAILED", "Memory inti failed");
        return RET_NULL;
    }
    memory->error = MERRY_ERROR_NONE;
    memory->number_of_pages = 0;
    memory->pages = (MerryDMemPage **)malloc(sizeof(MerryDMemPage *) * num_of_pages);
    if (memory->pages == RET_NULL)
    {
        // failed
        _log_(_MEM_, "FAILED", "Memory inti failed");
        free(memory);
        return RET_NULL;
    }
    // now we need to initialize every single page
    for (msize_t i = 0; i < num_of_pages; i++, memory->number_of_pages++)
    {
        _llog_(_MEM_, "ALLOCATING PAGES", "Allocating page %lu", i);
        memory->pages[i] = merry_mem_allocate_new_mempage_provided(mapped_pages[i]);
        if (memory->pages[i] == RET_NULL)
        {
            // failure
            _log_(_MEM_, "FAILED", "Memory intialization failed while allocating pages");
            merry_dmemory_free(memory);
            return RET_NULL;
        }
    }
    // we have allocated everything successfully
    _log_(_MEM_, "MEM_INIT_SUCCESS", "Memory successfully initialized");
    return memory;
}

void merry_dmemory_free(MerryDMemory *memory)
{
    _log_(_MEM_, "DESTORYING", "Destroying memory");
    if (surelyF(memory == NULL))
        return;
    if (memory->pages != NULL)
    {
        for (msize_t i = 0; i < memory->number_of_pages; i++)
        {
            merry_mem_free_mempage(memory->pages[i]);
        }
        free(memory->pages);
    }
    free(memory);
}

mret_t merry_dmemory_read_byte(MerryDMemory *memory, maddress_t address, mbptr_t _store_in)
{
    // get the actual address and the page
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    *_store_in = memory->pages[addr.page]->address_space[addr.offset];
    return RET_SUCCESS;
}

mret_t merry_dmemory_write_byte(MerryDMemory *memory, maddress_t address, mqword_t _to_write)
{
    // pretty much the same as read
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    memory->pages[addr.page]->address_space[addr.offset] = _to_write;
    return RET_SUCCESS;
}

mret_t merry_dmemory_read_lock(MerryDMemory *memory, maddress_t address, mqptr_t _store_in)
{
    // this is the same as read but with page locking ensuring that no other thread can access the same page.
    MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(_MERRY_MEMORY_IS_ACCESS_ERROR_(addr.offset)))
    {
        memory->error = MERRY_MEM_ACCESS_ERROR;
        return RET_FAILURE;
    }
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    merry_mutex_lock(memory->pages[addr.page]->lock);
    mqptr_t temp = &memory->pages[addr.page]->address_space[addr.offset];
    *_store_in = *temp;
    merry_mutex_unlock(memory->pages[addr.page]->lock);
    return RET_SUCCESS;
}

mret_t merry_dmemory_write_lock(MerryDMemory *memory, maddress_t address, mqword_t _to_write)
{
    MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(_MERRY_MEMORY_IS_ACCESS_ERROR_(addr.offset)))
    {
        memory->error = MERRY_MEM_ACCESS_ERROR;
        return RET_FAILURE;
    }
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    merry_mutex_lock(memory->pages[addr.page]->lock);
    mqptr_t temp = &memory->pages[addr.page]->address_space[addr.offset];
    *temp = _to_write; // write the value
    merry_mutex_unlock(memory->pages[addr.page]->lock);
    return RET_SUCCESS;
}

mptr_t merry_dmemory_get_address(MerryDMemory *memory, maddress_t address)
{
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    // this just basically returns an actual address to the address that the manager can use
    return &memory->pages[addr.page]->address_space[addr.offset];
}