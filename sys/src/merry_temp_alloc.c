#include "../memory/merry_temp_alloc.h"

/*Overseer*/
mret_t merry_temp_overseer_alloc(msize_t size)
{
    // this process is very very complicated in Windows
    merry_temp_start_address = _MERRY_GET_CURRENT_BRK_POINT_; // get the current break point
    merry_temp_current_pos = merry_temp_start_address;        // we haven't allocated any memory yet
    merry_temp_memory_in_use = 0;                             // no memory currently in use
    if (_MERRY_INC_MEMLIM_(size) == RET_NULL)
        return RET_FAILURE; // we failed
    // we succeeded if we reached here
    merry_temp_end_address = _MERRY_GET_CURRENT_BRK_POINT_; // get the end address
    merry_temp_managed_size = size;                         // Temp Allocator now has size bytes of memory to manage
    return RET_SUCCESS;
}