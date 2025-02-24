#ifndef _MERRY_INTERNALS_
#define _MERRY_INTERNALS_

// define the necessary configurations needed for the internal components here
#include <merry_config.h>
#include <merry_types.h>

// Configurations for memory
#define _MERRY_MEMORY_ADDRESSES_PER_PAGE_ 1048576                   // the number of addresses per page[Equal to 1MB]
#define _MERRY_MEMORY_QS_PER_PAGE_ 131072
#define _MERRY_MEMORY_GET_PAGE_OFFSET_(address) (address & 0xFFFFF) // get the offset from the address
#define _MERRY_MEMORY_GET_PAGE_(address) (address >> 20)            // get the page number from the address
// we currently have no limit set to how many pages Merry can have at max but lets leave it to the OS
#define _MERRY_MEMORY_GENERATE_ADDRESS_(page, offset) ((page << 20) & offset) // generate an address
// the memory follows the same endianness as the host system
#define _MERRY_MEMORY_BYTE_ORDER_ _MERRY_BYTE_ORDER_
#define _MERRY_MEMORY_IS_ACCESS_ERROR_(offset) ((offset + 7) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_)
#define _MERRY_STACKMEM_BYTE_LEN_ _MERRY_MEMORY_ADDRESSES_PER_PAGE_
#define _MERRY_STACKMEM_SIZE_ 131072 // this is the number of qwords and not the bytes[equals 1MB]

#define _MERRY_MAX_ADDRESS_ 131071

#endif