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
        goto allocate_new_block; // allocate new block
    else
    {
        // the free_list is not empty and we have something there
        // we first traverse through the list to see if we can find the block that meets the requirements
        block = merry_temp_get_first_fit(size);
        if (block == RET_NULL)
        {
            // we found none but that doesn't mean we failed
            // we can split a larger free block or merge two smaller blocks to get the needed free block
            // but first we need to check if the free list head only contains one block
            // since we got RET_NULL it only means these things: We have memory blocks smaller than size or we only have one free block
            if (allocator.free_list->next == allocator.free_list->prev)
            {
                // we only have one free block
                // we have to use this
                if (allocator.free_list->block_len > (size + _MERRY_TEMP_ALLOC_BLOCK_SIZE_))
                {
                    // if the remaining size after split is less than or equal to the metadata, we cannot make use of it
                    block = allocator.free_list; // block becomes that
                    __new_block = (mptr_t)(block + _MERRY_TEMP_ALLOC_BLOCK_SIZE_);
                    MerryTempAllocBlock *new_free_head = (MerryTempAllocBlock *)(__new_block + size); // the new head
                    new_free_head->next = new_free_head;
                    new_free_head->prev = new_free_head;
                    new_free_head->block_len = (block->block_len - size - _MERRY_TEMP_ALLOC_BLOCK_SIZE_);
                    allocator.free_list = new_free_head;
                    block->block_len = size;
                    merry_temp_update_alloclist(block);
                    goto end; // we have allocated the memory
                }
                else
                {
                    // we have one free block but it doesn't fulfill our requirments so we will have to create new blocks
                    goto allocate_new_block; // allocate new block
                }
                // we have memory blocks smaller than size
                // in this case we can merge two adjacent blocks to get the desired size
                // it is to be made sure that the blocks be adjacent and not seperated by another block
                MerryTempAllocBlock *temp = merry_temp_get_adjacent_free_blocks(size);
                // we have gotten what we wanted
                // but to preserve memory we cannot simply provide the whole block if it is larger than size
                if ((temp->block_len + temp->next->block_len + _MERRY_TEMP_ALLOC_BLOCK_SIZE_) == size)
                {
                    // this would happen rarely
                    block = temp;
                    __new_block = (mptr_t)(block + _MERRY_TEMP_ALLOC_BLOCK_SIZE_);
                    block->block_len = size;
                    // update the other blocks
                    if (temp == allocator.free_list)
                    {
                        if (temp->next->next != allocator.free_list)
                        {
                            // if temp is the head block and we have more than three free blocks
                            temp->prev->next = temp->next->next;
                            temp->next->next->prev = temp->prev;
                            allocator.free_list = temp->next->next; // update the head
                        }
                        else
                        {
                            // if temp is the head block and we have only two blocks
                            allocator.free_list = NULL; // no more free blocks
                        }
                    }
                    else if (temp->next == allocator.free_list)
                    {
                        // This situation may rarely occur but it still has non-zero possibility
                        // if this situation comes then it is guranted that
                        if (temp->next->next != allocator.free_list)
                        {
                            // if temp is the head block and we have more than three free blocks
                            temp->prev->next = temp->next->next;
                            temp->next->next->prev = temp->prev;
                            allocator.free_list = temp->next->next; // update the head
                        }
                        else
                        {
                            // if temp is the head block and we have only two blocks
                            allocator.free_list = NULL; // no more free blocks
                        }
                    }
                    merry_temp_update_alloclist(block);
                    goto end;
                }
                else
                {
                    // the size of the block is too big
                    block = temp;
                    __new_block = (mptr_t)(block + _MERRY_TEMP_ALLOC_BLOCK_SIZE_);
                    MerryTempAllocBlock *replacement = (MerryTempAllocBlock *)(__new_block + size); // the replacement block
                    replacement->next = temp->next->next;
                    replacement->prev = temp->prev;
                    // update other blocks
                    temp->prev->next = replacement;
                    temp->next->prev = replacement;
                    block->block_len = size;
                    merry_temp_update_alloclist(block);
                    goto end; // successfully allocated
                }
            }
        }
    }
end:
    merry_mutex_unlock(allocator.lock);
    return __new_block;
allocate_new_block:
    if (_MERRY_TEMP_GET_FREE_MEMSIZE_ < size)
    {
        // there are no free blocks and we have no memory
        if (merry_temp_overseer_increase_pool_size(size * 4) == RET_FAILURE)
            goto end; // we failed
    }
    // we now have memory and can allocate new block
    block = (MerryTempAllocBlock *)(_MERRY_TEMP_GET_CURRENT_POS_);                        // get the new block
    block->block_len = size;                                                              // the size of the block[We do not need the in use flag]
    __new_block = (mptr_t)(_MERRY_TEMP_GET_CURRENT_POS_ + _MERRY_TEMP_ALLOC_BLOCK_SIZE_); // set the new block's pointer
    // we now have to update the linked list
    merry_temp_update_alloclist(block);
    _MERRY_TEMP_UPDATE_MEM_USE_SIZE_(size); // update the memory usage
    goto end;
}