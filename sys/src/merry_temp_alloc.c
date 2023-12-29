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

void merry_temp_alloc_free()
{
    if (surelyF(allocator.lock == NULL))
        return;
    merry_mutex_destroy(allocator.lock);
    allocator.allocated_list = NULL;
    allocator.free_list = NULL;
}

mptr_t merry_temp_alloc(msize_t size)
{
    // we first need to make a head
    merry_mutex_lock(allocator.lock);
    mptr_t __new_block = NULL;
    size = merry_align_size(size); // get the aligned size
    if (size == 0)
        goto end; // failure
    // we need to allocate under two circumstances
    // 1. If we have free blocks
    // 2. If we do not have free blocks
    // first by second approach
    if (allocator.free_list == NULL)
    {
        // the free list is empty
        // so we need a new block
        MerryTempAllocBlock *new_block = merry_temp_allocate_new_block(size);
        if (new_block == RET_NULL)
            goto end; // we failed
        // set the pointer
        __new_block = (mptr_t)(new_block + _MERRY_TEMP_ALLOC_BLOCK_SIZE_);
        // this is a new block and hence we need to update the allocator list
        merry_temp_update_alloclist(new_block);
        _MERRY_TEMP_UPDATE_MEM_USE_SIZE_(size); // update the memory consumption size
        goto end;                               // we did everything successfully
    }
    // the free list is not empty and hence we have some memory we can recycle
    /*
     Here are the things that can happen:
     1. We have free blocks but they don't fulfill the requirement.
     2. We have free blocks and they fulfill the requirement.
     3. The free block is the head block
    */
    MerryTempAllocBlock *free_block = merry_temp_get_first_fit(size);
    if (free_block != RET_NULL && free_block == allocator.free_list)
    {
        // the fitting block is the head block
        __new_block = (mptr_t)(free_block + _MERRY_TEMP_ALLOC_BLOCK_SIZE_);
        if (free_block->block_len == size || (free_block->block_len > size && (free_block->block_len < (size + _MERRY_TEMP_ALLOC_BLOCK_SIZE_ + 16))))
        {
            if (free_block->next == free_block)
                allocator.free_list = NULL;
            else
            {
                free_block->next->prev = free_block->prev;
                free_block->prev->next = free_block->next;
            }
            merry_temp_update_alloclist(free_block);
            goto end;
        }
        else
        {
            // the block fulfills even the splitting condition
            MerryTempAllocBlock *new_head = (MerryTempAllocBlock *)(__new_block + size);
            new_head->next = free_block->next;
            new_head->prev = free_block->prev;
            new_head->block_len = free_block->block_len - size - _MERRY_TEMP_ALLOC_BLOCK_SIZE_;
            allocator.free_list = new_head;
            merry_temp_update_alloclist(free_block);
            goto end;
        }
    }
    if (free_block != RET_NULL)
    {
        // there are blocks that fulfil the requirement but there are more possibilities:
        /*
          The block may be larger than size or exactly equal to size.
        */
        __new_block = (mptr_t)(free_block + _MERRY_TEMP_ALLOC_BLOCK_SIZE_);
        if (free_block->block_len == size || (free_block->block_len > size && (free_block->block_len < (size + _MERRY_TEMP_ALLOC_BLOCK_SIZE_ + 16))))
        {
            free_block->next->prev = free_block->prev;
            free_block->prev->next = free_block->next;
            merry_temp_update_alloclist(free_block);
            goto end;
        }
        else
        {
            // the block fulfills even the splitting condition
            MerryTempAllocBlock *new_head = (MerryTempAllocBlock *)(__new_block + size);
            new_head->next = free_block->next;
            new_head->prev = free_block->prev;
            new_head->block_len = free_block->block_len - size - _MERRY_TEMP_ALLOC_BLOCK_SIZE_;
            allocator.free_list = new_head;
            merry_temp_update_alloclist(free_block);
            goto end;
        }
    }
end:
    merry_mutex_unlock(allocator.lock);
    return __new_block;
}