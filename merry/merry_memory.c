#include "internals/merry_memory.h"

// helper function: Allocate a new memory page and return it
_MERRY_INTERNAL_ MerryMemPage *merry_mem_allocate_new_mempage()
{
    MerryMemPage *new_page = (MerryMemPage *)malloc(sizeof(MerryMemPage));
    if (new_page == RET_NULL)
    {
        // failed allocation
        return RET_NULL;
    }
    // try allocating the address space
    if ((new_page->address_space = _MERRY_MEMORY_PGALLOC_MAP_PAGE_) == NULL)
    {
        free(new_page);
        return RET_NULL; // we failed
    }
    // initialize the page's lock
    if ((new_page->lock = merry_mutex_init()) == RET_NULL)
    {
        _MERRY_MEMORY_PGALLOC_UNMAP_PAGE_(new_page->address_space);
        free(new_page);
        return RET_NULL;
    }
    // everything went successfully
    return new_page;
}

_MERRY_INTERNAL_ MerryMemPage *merry_mem_allocate_new_mempage_provided(mqptr_t page)
{
    MerryMemPage *new_page = (MerryMemPage *)malloc(sizeof(MerryMemPage));
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
        _MERRY_MEMORY_PGALLOC_UNMAP_PAGE_(new_page->address_space);
        free(new_page);
        return RET_NULL;
    }
    // everything went successfully
    _log_(_MEM_, "Page Allocation", "Allocating memory provided");
    return new_page;
}

// helper function :free an allocate memory page
_MERRY_INTERNAL_ void merry_mem_free_mempage(MerryMemPage *page)
{
    if (surelyF(page == NULL))
        return;
    if (page->lock != NULL)
    {
        merry_mutex_destroy(page->lock);
    }
    if (surelyT(page->address_space != NULL))
    {
        _MERRY_MEMORY_PGALLOC_UNMAP_PAGE_(page->address_space);
    }
    free(page); // that is all
}

// exposed function: initialize memory with num_of_pages pages
MerryMemory *merry_memory_init(msize_t num_of_pages)
{
    _llog_(_MEM_, "INIT", "Intializing memory with %lu pages", num_of_pages);
    MerryMemory *memory = (MerryMemory *)malloc(sizeof(MerryMemory));
    if (memory == RET_NULL)
    {
        // we failed
        _log_(_MEM_, "FAILED", "Memory inti failed");
        return RET_NULL;
    }
    memory->error = MERRY_ERROR_NONE;
    memory->number_of_pages = 0;
    memory->pages = (MerryMemPage **)malloc(sizeof(MerryMemPage *) * num_of_pages);
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
            merry_memory_free(memory);
            return RET_NULL;
        }
    }
    // we have allocated everything successfully
    _log_(_MEM_, "MEM_INIT_SUCCESS", "Memory successfully initialized");
    return memory;
}

MerryMemory *merry_memory_init_provided(mqptr_t *mapped_pages, msize_t num_of_pages)
{
    // just perform the regular allocation but don't map new pages
    // instead use the already mapped ones
    _llog_(_MEM_, "INIT", "Intializing memory with %lu pages", num_of_pages);
    MerryMemory *memory = (MerryMemory *)malloc(sizeof(MerryMemory));
    if (memory == RET_NULL)
    {
        // we failed
        _log_(_MEM_, "FAILED", "Memory inti failed");
        return RET_NULL;
    }
    memory->error = MERRY_ERROR_NONE;
    memory->number_of_pages = 0;
    memory->pages = (MerryMemPage **)malloc(sizeof(MerryMemPage *) * num_of_pages);
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
            merry_memory_free(memory);
            return RET_NULL;
        }
    }
    // we have allocated everything successfully
    _log_(_MEM_, "MEM_INIT_SUCCESS", "Memory successfully initialized");
    return memory;
}

void merry_memory_free(MerryMemory *memory)
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

mret_t merry_memory_read(MerryMemory *memory, maddress_t address, mqptr_t _store_in)
{
    // get the actual address and the page
    MerryAddress addr = _MERRY_MEMORY_DEDUCE_ADDRESS_(address);
    // We read 8 bytes at once and so we have to check if the offset is within the page's limit
    // Since the CPU will read the memory in an aligned way, the program can access it in ways that are unpredictable
    // if the program requests the address that is at the end of a Page then getting the other 7 bytes would require getting it from another page
    // This is inconvenient and wrong. This type of access will generate an Access Error
    // we will expect even the program to behave correctly here
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
    mbptr_t temp = &memory->pages[addr.page]->address_space[addr.offset];
    // in LITTLE ENDIAN systems dereferencing temp will correctly get the next 7 bytes but in little endian format
    // in BIG ENDIAN systems dereferencing temp will correctly get the next 7 bytes and in the format that makes sense to humans
    // The VM is going to use whatever endianness the host has
    _mem_read_(temp, _store_in);
    return RET_SUCCESS;
}

mret_t merry_memory_write(MerryMemory *memory, maddress_t address, mqword_t _to_write)
{
    // pretty much the same as read
    MerryAddress addr = _MERRY_MEMORY_DEDUCE_ADDRESS_(address);
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
    mqptr_t temp = &memory->pages[addr.page]->address_space[addr.offset];
    // in LITTLE ENDIAN systems dereferencing temp will correctly get the next 7 bytes but in little endian format
    // in BIG ENDIAN systems dereferencing temp will correctly get the next 7 bytes and in the format that makes sense to humans
    // The VM is going to use whatever endianness the host has
    *temp = _to_write; // write the value
    return RET_SUCCESS;
}

mret_t merry_memory_read_lock(MerryMemory *memory, maddress_t address, mqptr_t _store_in)
{
    // this is the same as read but with page locking ensuring that no other thread can access the same page.
    MerryAddress addr = _MERRY_MEMORY_DEDUCE_ADDRESS_(address);
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
    // this is absurd as we are locking and unlocking for just one read and write
    // well we could use atomic operations but they are not fully understood so we are stuck to being inefficient
    // we can provide some optimization here
    // we can decrease the page size and make it such that the possibility of having the data of different variables in different pages is high
    // this can make the VM faster as multiple vcores can access different variables in different pages without blocking one another
    // we can add further optimization by making it such that the OS uses the non locked read/write functions in case we have only one vcore running
    // or we can add some feature such that any data that any two vcores are accessing is private to them and if we can make the Manager aware of that then
    // it can use the non-lock read//write functions instead
    merry_mutex_lock(memory->pages[addr.page]->lock);
    mqptr_t temp = &memory->pages[addr.page]->address_space[addr.offset];
    *_store_in = *temp;
    merry_mutex_unlock(memory->pages[addr.page]->lock);
    return RET_SUCCESS;
}

mret_t merry_memory_write_lock(MerryMemory *memory, maddress_t address, mqword_t _to_write)
{
    MerryAddress addr = _MERRY_MEMORY_DEDUCE_ADDRESS_(address);
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

// mret_t merry_memory_load(MerryMemory *memory, mqptr_t to_load, msize_t num_of_qs)
// {
//     // For optimization purposed, we will need to take some measures here
//     // as the size of the input file grows, the time to read and load grows by a lot[Almost exponentially]
//     // This lag in performance really shows itself ones we have programs of just a few kilobytes.
//     // we will need to use various methods in order to write to memory
//     // for num_of_qs less than 1 memory page size, we can copy the memory from to_load simply
// }

// // we also need to check if the requested page is being held by a core for atomic operations
// if (memory->pages[addr.page]->details._is_locked == mtrue)
// {
//     if (memory->pages[addr.page]->details._locker != key)
//     {
//         // the page is being held by some other core
//         // in this case we simply cannot return
//     }
//     else
//     {
//         // this core holds the memory page and so we can access it safely
//     }
// }