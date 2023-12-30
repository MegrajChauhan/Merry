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
        goto allocate_new_and_ret;
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
        /*
          To be splittable, the block must have the at least the size equivalent to requested size + an additional block capable of storing 2 qwords or 16 bytes
        */
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
        }
        else
        {
            // the block fulfills even the splitting condition
            MerryTempAllocBlock *new_head = (MerryTempAllocBlock *)(__new_block + size);
            new_head->next = free_block->next;
            new_head->prev = free_block->prev;
            new_head->block_len = free_block->block_len - size - _MERRY_TEMP_ALLOC_BLOCK_SIZE_;
            allocator.free_list = new_head;
        }
        merry_temp_update_alloclist(free_block);
        goto end;
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
        }
        else
        {
            // the block fulfills even the splitting condition
            MerryTempAllocBlock *replacement = (MerryTempAllocBlock *)(__new_block + size);
            replacement->next = free_block->next;
            replacement->prev = free_block->prev;
            free_block->next->prev = replacement;
            free_block->prev->next = replacement;
            replacement->block_len = free_block->block_len - size - _MERRY_TEMP_ALLOC_BLOCK_SIZE_;
        }
        merry_temp_update_alloclist(free_block);
        goto end;
    }
    // we now have to resort to merging the blocks
    // we look for adjacent free blocks and then use them
    // It is to be noted that we only use 2 adjacent blocks but not three or more for simplicity
    free_block = merry_temp_get_adjacent_free_blocks(size);
    // we can find or we cannot
    if (free_block == RET_NULL)
    {
        // we didn't find any two adjacent fitting blocks
        // so we need to allocate a new block
        goto allocate_new_and_ret;
    }
    else
    {
        // we did find two adjacent free blocks
        // given how we search for two adjacent free blocks, we may be assured that the first of the two blocks will never be the head block
        // given this we need to address when the second of the free blocks is the head block
        if (free_block->next == allocator.free_list)
        {
            // we have to either split it or allocate the entire two blocks
            __new_block = (mptr_t)(free_block + _MERRY_TEMP_ALLOC_BLOCK_SIZE_); // set the pointer first
            // we have to see if the blocks are splittable or not
            register msize_t temp = free_block->block_len + free_block->next->block_len;
            if (temp == size || (temp > size && (temp < (size + 2 * _MERRY_TEMP_ALLOC_BLOCK_SIZE_ + 16))))
            {
                // the blocks are not splittable
                // we use the entire two blocks for this allocation
                // since we are in this condition it means that the first free block is the tail
                if (allocator.free_list->next == free_block)
                {
                    // if we only have 2 free blocks in the free list
                    allocator.free_list = NULL;
                }
                else
                {
                    // we have more than that
                    allocator.free_list->next->prev = free_block->prev;
                    allocator.free_list = allocator.free_list->next;
                }
                free_block->block_len += free_block->next->block_len + _MERRY_TEMP_ALLOC_BLOCK_SIZE_;
            }
            else
            {
                // the blocks are splittable
                // this will replace the head
                MerryTempAllocBlock *new_head = (MerryTempAllocBlock *)(__new_block + size);
                new_head->next = allocator.free_list->next;
                new_head->prev = allocator.free_list->prev->prev;
                new_head->block_len = free_block->block_len - size - _MERRY_TEMP_ALLOC_BLOCK_SIZE_;
                allocator.free_list = new_head;
            }
            merry_temp_update_alloclist(free_block);
            goto end;
        }
        // the two free blocks do not contain the head block hence we can to some SPLIT checking
        __new_block = (mptr_t)(free_block + _MERRY_TEMP_ALLOC_BLOCK_SIZE_); // set the pointer first
        // we have to see if the blocks are splittable or not
        register msize_t temp = free_block->block_len + free_block->next->block_len;
        if (temp == size || (temp > size && (temp < (size + 2 * _MERRY_TEMP_ALLOC_BLOCK_SIZE_ + 16))))
        {
            // the blocks are not splittable
            // we use the entire two blocks for this allocation
            free_block->prev->next = free_block->next->next;
            free_block->next->prev = free_block->prev;
            free_block->block_len += free_block->next->block_len + _MERRY_TEMP_ALLOC_BLOCK_SIZE_;
        }
        else
        {
            // the blocks are splittable
            MerryTempAllocBlock *replacement = (MerryTempAllocBlock *)(__new_block + size);
            replacement->block_len = free_block->block_len - size - _MERRY_TEMP_ALLOC_BLOCK_SIZE_;
            replacement->next = free_block->next->next;
            replacement->prev = free_block->prev;
            free_block->next->next->prev = replacement;
            free_block->prev->next = replacement;
        }
        merry_temp_update_alloclist(free_block);
        goto end;
    }
    // at this point, __new_block should be initialized and the function should return
end:
    merry_mutex_unlock(allocator.lock);
    return __new_block;
allocate_new_and_ret:
    MerryTempAllocBlock *new_block = merry_temp_allocate_new_block(size);
    if (new_block == RET_NULL)
        goto end; // we failed
    // set the pointer
    __new_block = (mptr_t)(new_block + _MERRY_TEMP_ALLOC_BLOCK_SIZE_);
    // this is a new block and hence we need to update the allocator list
    merry_temp_update_alloclist(new_block);
    _MERRY_TEMP_UPDATE_MEM_USE_SIZE_(size); // update the memory consumption size
    goto end;
}

/*
 This will free the block.BELIEVE THAT THE PROGRAM WILL NOT MESS WITH THE METADATA
 */
void merry_temp_free(mptr_t _ptr)
{
    if (surelyF(_ptr == NULL))
        return;
    merry_mutex_lock(allocator.lock);
    MerryTempAllocBlock *block = (MerryTempAllocBlock *)(_ptr - _MERRY_TEMP_ALLOC_BLOCK_SIZE_);
    // check if block is the head
    if (block == allocator.allocated_list)
    {
        // this is the head
        allocator.allocated_list = NULL;
        merry_temp_update_freelist(block);
    }
    else
    {
        // it is not the head block
        block->next->prev = block->prev;
        block->prev->next = block->next;
        merry_temp_update_freelist(block);
    }
    _ptr = NULL; // it cannot access this memory again
    merry_mutex_unlock(allocator.lock);
}