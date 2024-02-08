#include "../merry_input.h"

mret_t merry_read_char(MerryDMemory *mem, maddress_t address)
{
    mbptr_t _store_in;
    if ((_store_in = merry_dmemory_get_byte_address(mem, address)) == RET_NULL)
        return RET_FAILURE;
    *_store_in = getchar();
    return RET_SUCCESS;
}