#include "../merry_output.h"

mret_t merry_write_char(MerryDMemory *mem, maddress_t address)
{
    mbptr_t _to_write;
    if ((_to_write = merry_dmemory_get_byte_address(mem, address)) == RET_NULL)
        return RET_FAILURE;
    putchar(*_to_write);
    return RET_SUCCESS;
}