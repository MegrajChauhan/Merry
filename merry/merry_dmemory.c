#include "merry_dmemory.h"

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
    // everything went successfully
    return new_page;
}

_MERRY_INTERNAL_ MerryDMemPage *merry_mem_allocate_new_mempage_provided(mbptr_t page)
{
    MerryDMemPage *new_page = (MerryDMemPage *)malloc(sizeof(MerryDMemPage));
    if (new_page == RET_NULL)
    {
        // failed allocation
        return RET_NULL;
    }
    // try allocating the address space
    new_page->address_space = page; // we were provided
    // everything went successfully
    return new_page;
}

// helper function :free an allocate memory page
_MERRY_INTERNAL_ void merry_mem_free_mempage(MerryDMemPage *page)
{
    if (surelyF(page == NULL))
        return;
    if (surelyT(page->address_space != NULL))
    {
        _MERRY_DMEMORY_PGALLOC_UNMAP_PAGE_(page->address_space);
    }
    free(page); // that is all
}

// exposed function: initialize memory with num_of_pages pages
MerryDMemory *merry_dmemory_init(msize_t num_of_pages)
{
    MerryDMemory *memory = (MerryDMemory *)malloc(sizeof(MerryDMemory));
    if (memory == RET_NULL)
    {
        // we failed
        return RET_NULL;
    }
    memory->error = MERRY_ERROR_NONE;
    memory->number_of_pages = 0;
    memory->pages = (MerryDMemPage **)malloc(sizeof(MerryDMemPage *) * num_of_pages);
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
            merry_dmemory_free(memory);
            return RET_NULL;
        }
    }
    // we have allocated everything successfully
    return memory;
}

mret_t merry_dmemory_add_new_page(MerryDMemory *memory)
{
    MerryDMemPage **temp = memory->pages;
    if ((memory->pages = (MerryDMemPage **)realloc(memory->pages, sizeof(MerryDMemPage *) * (memory->number_of_pages + 1))) == NULL)
    {
        memory->pages = temp; // restore on fail
        return RET_FAILURE;
    }
    memory->pages[memory->number_of_pages] = merry_mem_allocate_new_mempage();
    if (memory->pages[memory->number_of_pages] == RET_NULL)
        return RET_FAILURE;
    memory->number_of_pages++;
    return RET_SUCCESS;
}

MerryDMemory *merry_dmemory_init_provided(mbptr_t *mapped_pages, msize_t num_of_pages)
{
    // just perform the regular allocation but don't map new pages
    // instead use the already mapped ones
    MerryDMemory *memory = (MerryDMemory *)malloc(sizeof(MerryDMemory));
    if (memory == RET_NULL)
    {
        // we failed
        return RET_NULL;
    }
    memory->error = MERRY_ERROR_NONE;
    memory->number_of_pages = 0;
    memory->pages = (MerryDMemPage **)malloc(sizeof(MerryDMemPage *) * num_of_pages);
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
            merry_dmemory_free(memory);
            return RET_NULL;
        }
    }
    // we have allocated everything successfully
    return memory;
}

void merry_dmemory_free(MerryDMemory *memory)
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

mret_t merry_dmemory_read_byte(MerryDMemory *memory, maddress_t address, mqptr_t _store_in)
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

mret_t merry_dmemory_read_byte_atm(MerryDMemory *memory, maddress_t address, mqptr_t _store_in)
{
    // get the actual address and the page
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    *_store_in = atomic_load(&memory->pages[addr.page]->address_space[addr.offset]);
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

mret_t merry_dmemory_write_byte_atm(MerryDMemory *memory, maddress_t address, mqword_t _to_write)
{
    // pretty much the same as read
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    atomic_store(&memory->pages[addr.page]->address_space[addr.offset], _to_write & 0xFF);
    return RET_SUCCESS;
}

mret_t merry_dmemory_read_word(MerryDMemory *memory, maddress_t address, mqptr_t _store_in)
{
    // get the actual address and the page
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    // The byte that the address points to is the lower byte while the next byte is the
    if (surelyF((addr.offset + 1) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    *_store_in = *((mwptr_t)(memory->pages[addr.page]->address_space + addr.offset));
    return RET_SUCCESS;
}

mret_t merry_dmemory_read_word_atm(MerryDMemory *memory, maddress_t address, mqptr_t _store_in)
{
    // get the actual address and the page
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    // The byte that the address points to is the lower byte while the next byte is the
    if (surelyF((addr.offset + 1) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    *_store_in = (mqword_t)atomic_load(((mwptr_t)(memory->pages[addr.page]->address_space + addr.offset)));
    return RET_SUCCESS;
}

mret_t merry_dmemory_write_word(MerryDMemory *memory, maddress_t address, mqword_t _to_write)
{
    // pretty much the same as read
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    if (surelyF((addr.offset + 1) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    *((mwptr_t)(memory->pages[addr.page]->address_space + addr.offset)) = _to_write & 0xFFFF;
    return RET_SUCCESS;
}

mret_t merry_dmemory_write_word_atm(MerryDMemory *memory, maddress_t address, mqword_t _to_write)
{
    // pretty much the same as read
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    if (surelyF((addr.offset + 1) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    atomic_store((mwptr_t)(memory->pages[addr.page]->address_space + addr.offset), _to_write & 0xFFFF);
    return RET_SUCCESS;
}

mret_t merry_dmemory_read_dword(MerryDMemory *memory, maddress_t address, mqptr_t _store_in)
{
    // get the actual address and the page
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    // The byte that the address points to is the lower byte while the next byte is the
    if (surelyF((addr.offset + 3) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    *_store_in = *((mdptr_t)(memory->pages[addr.page]->address_space + addr.offset));
    return RET_SUCCESS;
}

mret_t merry_dmemory_read_dword_atm(MerryDMemory *memory, maddress_t address, mqptr_t _store_in)
{
    // get the actual address and the page
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    // The byte that the address points to is the lower byte while the next byte is the
    if (surelyF((addr.offset + 3) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    *_store_in = (mqword_t)atomic_load(&memory->pages[addr.page]->address_space + addr.offset);
    return RET_SUCCESS;
}

mret_t merry_dmemory_write_dword(MerryDMemory *memory, maddress_t address, mqword_t _to_write)
{
    // pretty much the same as read
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    if (surelyF((addr.offset + 3) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    *((mdptr_t)(memory->pages[addr.page]->address_space + addr.offset)) = _to_write & 0xFFFFFFFF;
    return RET_SUCCESS;
}

mret_t merry_dmemory_write_dword_atm(MerryDMemory *memory, maddress_t address, mqword_t _to_write)
{
    // pretty much the same as read
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    if (surelyF((addr.offset + 3) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    atomic_store((mdptr_t)(memory->pages[addr.page]->address_space + addr.offset), _to_write & 0xFFFFFFFF);
    return RET_SUCCESS;
}

mret_t merry_dmemory_read_qword(MerryDMemory *memory, maddress_t address, mqptr_t _store_in)
{
    // get the actual address and the page
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    // The byte that the address points to is the lower byte while the next byte is the
    if (surelyF((addr.offset + 7) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    *_store_in = *((mqptr_t)(memory->pages[addr.page]->address_space + addr.offset));
    return RET_SUCCESS;
}

mret_t merry_dmemory_read_qword_atm(MerryDMemory *memory, maddress_t address, mqptr_t _store_in)
{
    // get the actual address and the page
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    // The byte that the address points to is the lower byte while the next byte is the
    if (surelyF((addr.offset + 7) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    *_store_in = atomic_load((mqptr_t)(memory->pages[addr.page]->address_space + addr.offset));
    return RET_SUCCESS;
}

mret_t merry_dmemory_write_qword(MerryDMemory *memory, maddress_t address, mqword_t _to_write)
{
    // pretty much the same as read
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    if (surelyF((addr.offset + 7) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    *((mqptr_t)(memory->pages[addr.page]->address_space + addr.offset)) = _to_write;
    return RET_SUCCESS;
}

mret_t merry_dmemory_write_qword_atm(MerryDMemory *memory, maddress_t address, mqword_t _to_write)
{
    // pretty much the same as read
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    if (surelyF((addr.offset + 7) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    atomic_store((mqptr_t)(memory->pages[addr.page]->address_space + addr.offset), _to_write);
    return RET_SUCCESS;
}

mbptr_t merry_dmemory_get_byte_address(MerryDMemory *memory, maddress_t address)
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

mbptr_t merry_dmemory_get_byte_address_bounds(MerryDMemory *memory, maddress_t address, msize_t bound)
{
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    if (surelyF((addr.offset + bound) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    // this just basically returns an actual address to the address that the manager can use
    return &memory->pages[addr.page]->address_space[addr.offset];
}

mwptr_t merry_dmemory_get_word_address(MerryDMemory *memory, maddress_t address)
{
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    // this just basically returns an actual address to the address that the manager can use
    return (mwptr_t)(memory->pages[addr.page]->address_space + addr.offset);
}

// here bound is bound*2 bytes
mwptr_t merry_dmemory_get_word_address_bounds(MerryDMemory *memory, maddress_t address, msize_t bound)
{
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    if (surelyF((addr.offset + bound * 2) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    // this just basically returns an actual address to the address that the manager can use
    return (mwptr_t)(memory->pages[addr.page]->address_space + addr.offset);
}

mdptr_t merry_dmemory_get_dword_address(MerryDMemory *memory, maddress_t address)
{
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    // this just basically returns an actual address to the address that the manager can use
    return (mdptr_t)(memory->pages[addr.page]->address_space + addr.offset);
}

// here bound is bound*4 bytes
mdptr_t merry_dmemory_get_dword_address_bounds(MerryDMemory *memory, maddress_t address, msize_t bound)
{
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    if (surelyF((addr.offset + bound * 4) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    // this just basically returns an actual address to the address that the manager can use
    return (mdptr_t)(memory->pages[addr.page]->address_space + addr.offset);
}

mqptr_t merry_dmemory_get_qword_address(MerryDMemory *memory, maddress_t address)
{
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    // this just basically returns an actual address to the address that the manager can use
    return (mqptr_t)(memory->pages[addr.page]->address_space + addr.offset);
}

// here bound is bound*8 bytes
mqptr_t merry_dmemory_get_qword_address_bounds(MerryDMemory *memory, maddress_t address, msize_t bound)
{
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    if (surelyF((addr.offset + bound * 8) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    // this just basically returns an actual address to the address that the manager can use
    return (mqptr_t)(memory->pages[addr.page]->address_space + addr.offset);
}

mstr_t merry_dmemory_get_bytes_maybe_over_multiple_pages(MerryDMemory *memory, maddress_t address, msize_t length)
{
    mstr_t array = (mstr_t)malloc(length + 1);
    mstr_t curr = array;
    if (array == NULL)
        return RET_NULL;
    array[length] = '\0';
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        // this implies the request is for a page that doesn't exist
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_NULL;
    }
    msize_t dist_from_end = _MERRY_MEMORY_ADDRESSES_PER_PAGE_ - addr.offset;
    if (dist_from_end > length)
    {
        // the entire array is in this page only
        // simply copy the memory
        memcpy(array, memory->pages[addr.page]->address_space + addr.offset, length);
        return array; // we are entrusting the stdlib to not fail as this should not fail
    }
    else
    {
        // the array is in multiple pages
        // first copy whatever part is in this page
        curr += dist_from_end;
        memcpy(array, memory->pages[addr.page]->address_space + addr.offset, dist_from_end);
        length -= dist_from_end;
        // now until we copy everything, keep doing it
        register msize_t no_of_pages = length / _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
        register msize_t remaining = length % _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
        if ((addr.page + no_of_pages + (remaining > 0 ? 1 : 0)) >= memory->number_of_pages)
        {
            memory->error = MERRY_MEM_INVALID_ACCESS;
            free(array);
            return RET_NULL;
        }
        addr.page++;
        for (msize_t i = 0; i < no_of_pages; i++, addr.page++)
        {
            memcpy(curr, memory->pages[addr.page]->address_space, _MERRY_MEMORY_ADDRESSES_PER_PAGE_);
            curr += _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
        }
        if (remaining > 0)
        {
            // addr.page will be correctly pointed
            memcpy(curr, memory->pages[addr.page]->address_space, remaining);
        }
    }
    return array;
}

mret_t merry_dmemory_write_bytes_maybe_over_multiple_pages(MerryDMemory *memory, maddress_t address, msize_t length, mbptr_t array)
{
    // Exact opposite of the above
    if (array == NULL)
        return RET_SUCCESS;
    mbptr_t curr = array;
    register MerryDAddress addr = _MERRY_DMEMORY_DEDUCE_ADDRESS_(address);
    if (surelyF(addr.page >= memory->number_of_pages))
    {
        memory->error = MERRY_MEM_INVALID_ACCESS;
        return RET_FAILURE;
    }
    msize_t dist_from_end = _MERRY_MEMORY_ADDRESSES_PER_PAGE_ - addr.offset;
    if (dist_from_end > length)
    {
        memcpy(memory->pages[addr.page]->address_space + addr.offset, array, length);
        return RET_SUCCESS; // we are entrusting the stdlib to not fail as this should not fail
    }
    else
    {
        curr += dist_from_end;
        memcpy(memory->pages[addr.page]->address_space + addr.offset, array, dist_from_end);
        length -= dist_from_end;
        // now until we copy everything, keep doing it
        register msize_t no_of_pages = length / _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
        register msize_t remaining = length % _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
        if ((addr.page + no_of_pages + (remaining > 0 ? 1 : 0)) >= memory->number_of_pages)
        {
            memory->error = MERRY_MEM_INVALID_ACCESS;
            return RET_FAILURE;
        }
        addr.page++;
        for (msize_t i = 0; i < no_of_pages; i++, addr.page++)
        {
            memcpy(memory->pages[addr.page]->address_space, curr, _MERRY_MEMORY_ADDRESSES_PER_PAGE_);
            curr += _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
        }
        if (remaining > 0)
        {
            // addr.page will be correctly pointed
            memcpy(memory->pages[addr.page]->address_space, curr, remaining);
        }
    }
    return RET_SUCCESS;
}
