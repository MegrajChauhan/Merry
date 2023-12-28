#include "../memory/merry_temp_alloc.h"

/*Overseer*/
void *merry_temp_overseer_alloc(msize_t size)
{
    // this process is very very complicated in Windows
    merry_temp_start_address = _MERRY_GET_CURRENT_BRK_POINT_; // get the current break point
    register msize_t temp = merry_align_size(size);
    if (_MERRY_INC_MEMLIM_(temp) == RET_NULL)
        return RET_NULL; // we failed
    // we succeeded if we reached here
    merry_temp_end_address = _MERRY_GET_CURRENT_BRK_POINT_; // get the end address
    merry_temp_managed_size = temp;                         // Temp Allocator now has size bytes of memory to manage
    return (mqptr_t)(merry_temp_start_address);
}

void merry_temp_overseer_free()
{
    if (surelyF(merry_temp_managed_size == 0))
        return; // we are not managing any memory
    merry_temp_end_address = NULL;
    merry_temp_start_address = NULL;
    // that is all
}

mret_t merry_temp_overseer_increase_pool_size(msize_t inc_size)
{
    // this is called after alloc has been successful and hence we can continue safely
    inc_size = merry_align_size(inc_size);
    if (_MERRY_INC_MEMLIM_(inc_size) == RET_NULL)
        return RET_FAILURE; // we failed
    // we succeed
    merry_temp_end_address = _MERRY_GET_CURRENT_BRK_POINT_; // update the pointers
    merry_temp_managed_size += inc_size;
    return RET_SUCCESS; // we did everything correctly
}

mret_t merry_temp_alloc_init(msize_t init_size)
{
    if (surelyF(init_size == 0))
        return RET_FAILURE;
    // we need to initialize the overseer
    if (merry_temp_overseer_alloc(init_size) == RET_NULL)
        return RET_FAILURE; // we failed
    allocator.lock = merry_mutex_init();
    if (allocator.lock == RET_NULL)
        return RET_FAILURE; // we failed
    return RET_SUCCESS;     // we succeeded
}

mptr_t merry_temp_alloc(msize_t size)
{
    // we first need to make a head
    merry_mutex_lock(allocator.lock);
    mptr_t __new_block = NULL;
    size = merry_align_size(size); // get the aligned size
    if (size == 0)
        goto end; // failure
    // we also have to see if the current available memory can hold it or not
    // we check this when their are no free blocks else we can simply merge the free blocks or split them to fulfil the request
    MerryTempAllocBlock *block;
    if (allocator.free_list == NULL)
    {
        // if we do not have any free blocks
        // this maybe because this is the first allocation or that no block has been freed
        if (_MERRY_TEMP_GET_FREE_MEMSIZE_ < size)
        {
            // there are no free blocks and we have no memory
            if (merry_temp_overseer_increase_pool_size(size * 4) == RET_FAILURE)
                goto end; // we failed
        }
        // we now have memory and can allocate new block
        block = (MerryTempAllocBlock *)(_MERRY_TEMP_GET_CURRENT_POS_);                        // get the new block
        block->block_len = (1 << 63) & size;                                                  // the size of the block[The size will never fill the 64th bit so we will use it as the flag]
        __new_block = (mptr_t)(_MERRY_TEMP_GET_CURRENT_POS_ + _MERRY_TEMP_ALLOC_BLOCK_SIZE_); // set the new block's pointer
        // we now have to update the linked list
    }
end:
    merry_mutex_unlock(allocator.lock);
    return __new_block;
}