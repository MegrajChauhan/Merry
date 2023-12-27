#include "../memory/merry_temp_alloc.h"

/*Overseer*/
void *merry_temp_overseer_alloc(msize_t size)
{
    // this process is very very complicated in Windows
    merry_temp_start_address = _MERRY_GET_CURRENT_BRK_POINT_; // get the current break point
    if (_MERRY_INC_MEMLIM_(size) == RET_NULL)
        return RET_NULL; // we failed
    // we succeeded if we reached here
    merry_temp_end_address = _MERRY_GET_CURRENT_BRK_POINT_; // get the end address
    merry_temp_managed_size = size;                         // Temp Allocator now has size bytes of memory to manage
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
    if (_MERRY_INC_MEMLIM_(inc_size) == RET_NULL)
        return RET_FAILURE; // we failed
    // we succeed
    merry_temp_end_address = _MERRY_GET_CURRENT_BRK_POINT_; // update the pointers
    merry_temp_managed_size += inc_size;
    return RET_SUCCESS; // we did everything correctly
}
