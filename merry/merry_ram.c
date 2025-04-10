#include "merry_ram.h"

MerryRAM *merry_create_RAM(msize_t number_of_pages)
{
    merry_assert(number_of_pages != 0);

    MerryRAM *ram = (MerryRAM *)malloc(sizeof(MerryRAM));

    if (!ram)
        return RET_NULL;

    MerryNormalMemoryPage **pages = (MerryNormalMemoryPage **)malloc(sizeof(MerryNormalMemoryPage *) * number_of_pages);
    if (!pages)
    {
        free(ram);
        return RET_NULL;
    }

    msize_t i = 0;
    for (; i < number_of_pages; i++)
    {
        if ((pages[i] = merry_create_normal_memory_page()) == RET_NULL)
            goto __rid_of_during_error;
    }

    ram->pages = pages;
    ram->page_count = number_of_pages;
    if (merry_mutex_init(&ram->lock) == RET_FAILURE)
        goto __rid_of_during_error;

    ram->state.origin = _MERRY_ORIGIN_NONE_;
    return ram;
__rid_of_during_error: // not the best names :(
    for (msize_t j = 0; j < i; j++)
    {
        merry_return_normal_memory_page(pages[j]);
    }
    free(pages);
    free(ram);
    return RET_NULL;
}

mret_t merry_RAM_add_pages(MerryRAM *ram, msize_t num)
{
    // if failed, we won't invalidate the current page buffer
    merry_check_ptr(ram);
    merry_check_ptr(ram->pages);

    if (surelyF(num == 0))
        return RET_SUCCESS;

    register msize_t temp = ram->page_count + num;
    MerryNormalMemoryPage **pages = (MerryNormalMemoryPage **)malloc(sizeof(MerryNormalMemoryPage *) * temp);
    if (!pages)
    {
        merry_assign_state(ram->state, _MERRY_INTERNAL_SYSTEM_ERROR_, _MERRY_MEM_ALLOCATION_FAILURE_);
        return RET_NULL;
    }

    msize_t i = 0;
    for (i = ram->page_count; i < temp; i++)
    {
        if ((pages[i] = merry_create_normal_memory_page()) == RET_NULL)
            goto __rid_of_during_error;
    }

    memcpy(pages, ram->pages, sizeof(MerryNormalMemoryPage *) * ram->page_count);
    ram->page_count = temp;
    free(ram->pages);
    ram->pages = pages;
    return RET_SUCCESS;

__rid_of_during_error:
    for (msize_t j = ram->page_count; j < i; j++)
    {
        merry_return_normal_memory_page(pages[j]);
    }
    merry_obtain_memory_interface_state(&ram->state);
    free(pages);
    return RET_FAILURE;
}

mret_t merry_RAM_read_byte(MerryRAM *ram, maddress_t address, mbptr_t store_in)
{
    merry_check_ptr(ram);
    merry_check_ptr(ram->pages);
    merry_check_ptr(store_in);
    merry_assert(ram->page_count != 0);

    register msize_t page_num;
    register msize_t page_off;

    merry_deduce_address(address, page_num, page_off);

    if (surelyF(page_num >= ram->page_count))
    {
        merry_assign_state(ram->state, _MERRY_INTERNAL_SYSTEM_ERROR_, _MERRY_PAGE_FAULT_);
        return RET_FAILURE;
    }

    *store_in = ram->pages[page_num]->buf[page_off];
    return RET_SUCCESS;
}

mret_t merry_RAM_read_word(MerryRAM *ram, maddress_t address, mwptr_t store_in)
{
    merry_check_ptr(ram);
    merry_check_ptr(ram->pages);
    merry_check_ptr(store_in);
    merry_assert(ram->page_count != 0);
    
    register msize_t page_num;
    register msize_t page_off;
    
    merry_deduce_address(address, page_num, page_off);
    
    if (surelyF(page_num >= ram->page_count))
    {
        merry_assign_state(ram->state, _MERRY_INTERNAL_SYSTEM_ERROR_, _MERRY_PAGE_FAULT_);
        return RET_FAILURE;
    }

    // we have an extra check to make here
    // if the offset is right at the end of the current page then that implies the next byte
    // is on the next page and that means double reading
    
    *store_in = ram->pages[page_num]->buf[page_off];
    return RET_SUCCESS;

}

mret_t merry_RAM_read_dword(MerryRAM *ram, maddress_t address, mdptr_t store_in);

mret_t merry_RAM_read_qword(MerryRAM *ram, maddress_t address, mqptr_t store_in);

mret_t merry_RAM_write_byte(MerryRAM *ram, maddress_t address, mbyte_t value);

mret_t merry_RAM_write_word(MerryRAM *ram, maddress_t address, mword_t value);

mret_t merry_RAM_write_dword(MerryRAM *ram, maddress_t address, mdword_t value);

mret_t merry_RAM_write_qword(MerryRAM *ram, maddress_t address, mqword_t value);

void merry_destroy_RAM(MerryRAM *ram)
{
    merry_check_ptr(ram);
    merry_check_ptr(ram->pages);

    for (msize_t i = 0; i < ram->page_count; i++)
    {
        merry_check_ptr(ram->pages[i]);
        merry_return_normal_memory_page(ram->pages[i]);
    }
    merry_mutex_destroy(ram->lock);
    free(ram->pages);
    free(ram);
}