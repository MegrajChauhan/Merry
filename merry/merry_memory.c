#include "merry_memory.h"

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
    if ((new_page->address_space = (mqptr_t)_MERRY_MEMORY_PGALLOC_MAP_PAGE_) == NULL)
    {
        free(new_page);
        return RET_NULL; // we failed
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
    new_page->address_space = (mqptr_t)page; // we were provided
    // everything went successfully
    return new_page;
}

// helper function :free an allocate memory page
_MERRY_INTERNAL_ void merry_mem_free_mempage(MerryMemPage *page)
{
    if (surelyF(page == NULL))
        return;
    if (surelyT(page->address_space != NULL))
    {
        _MERRY_MEMORY_PGALLOC_UNMAP_PAGE_(page->address_space);
    }
    free(page); // that is all
}

// exposed function: initialize memory with num_of_pages pages
MerryMemory *merry_memory_init(msize_t num_of_pages)
{
    MerryMemory *memory = (MerryMemory *)malloc(sizeof(MerryMemory));
    if (memory == RET_NULL)
    {
        // we failed
        return RET_NULL;
    }
    memory->error = MERRY_ERROR_NONE;
    memory->number_of_pages = 0;
    memory->pages = (MerryMemPage **)malloc(sizeof(MerryMemPage *) * num_of_pages);
    if (memory->pages == RET_NULL)
    {
        // failed
        free(memory);
        return RET_NULL;
    }
    // now we need to initialize every single page
    for (msize_t i = 0; i < num_of_pages; i++, memory->number_of_pages++)
    {
        memory->pages[i] = merry_mem_allocate_new_mempage();
        if (memory->pages[i] == RET_NULL)
        {
            // failure
            merry_memory_free(memory);
            return RET_NULL;
        }
    }
    // we have allocated everything successfully
    return memory;
}

MerryMemory *merry_memory_init_provided(mqptr_t *mapped_pages, msize_t num_of_pages)
{
    // just perform the regular allocation but don't map new pages
    // instead use the already mapped ones
    MerryMemory *memory = (MerryMemory *)malloc(sizeof(MerryMemory));
    if (memory == RET_NULL)
    {
        // we failed
        return RET_NULL;
    }
    memory->error = MERRY_ERROR_NONE;
    memory->number_of_pages = 0;
    memory->pages = (MerryMemPage **)malloc(sizeof(MerryMemPage *) * num_of_pages);
    if (memory->pages == RET_NULL)
    {
        // failed
        free(memory);
        return RET_NULL;
    }
    // now we need to initialize every single page
    for (msize_t i = 0; i < num_of_pages; i++, memory->number_of_pages++)
    {
        memory->pages[i] = merry_mem_allocate_new_mempage_provided(mapped_pages[i]);
        if (memory->pages[i] == RET_NULL)
        {
            // failure
            merry_memory_free(memory);
            return RET_NULL;
        }
    }
    // we have allocated everything successfully
    return memory;
}

void merry_memory_free(MerryMemory *memory)
{
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
    register MerryAddress addr = _MERRY_MEMORY_DEDUCE_ADDRESS_(address);
    // We read 8 bytes at once and so we have to check if the offset is within the page's limit
    // Since the CPU will read the memory in an aligned way, the program can access it in ways that are unpredictable
    // if the program requests the address that is at the end of a Page then getting the other 7 bytes would require getting it from another page
    // This is inconvenient and wrong. This type of access will generate an Access Error
    // we will expect even the program to behave correctly here
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    *_store_in = memory->pages[addr.page]->address_space[addr.offset];
    // in LITTLE ENDIAN systems dereferencing temp will correctly get the next 7 bytes but in little endian format
    // in BIG ENDIAN systems dereferencing temp will correctly get the next 7 bytes and in the format that makes sense to humans
    // The VM is going to use whatever endianness the host has
    return RET_SUCCESS;
}

mret_t merry_memory_write(MerryMemory *memory, maddress_t address, mqword_t _to_write)
{
    // pretty much the same as read
    register MerryAddress addr = _MERRY_MEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    memory->pages[addr.page]->address_space[addr.offset] = _to_write;
    return RET_SUCCESS;
}

mptr_t merry_memory_get_address(MerryMemory *memory, maddress_t address)
{
    register MerryAddress addr = _MERRY_MEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    // this just basically returns an actual address to the address that the manager can use
    return &memory->pages[addr.page]->address_space[addr.offset];
}