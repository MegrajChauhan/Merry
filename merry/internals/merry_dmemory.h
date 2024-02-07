#ifndef _MERRY_DMEMORY_
#define _MERRY_DMEMORY_

#include "merry_internals.h"
#include "../../sys/merry_mem.h"
#include "../../sys/merry_thread.h" // memory needs to be thread safe
// #include "../lib/include/merry_memory_allocator.h" <LEGACY>
#include "../includes/merry_errors.h"
#include "../../utils/merry_logger.h"
#include "imp/merry_extra.h"
#include <stdlib.h>

typedef struct MerryDMemPage MerryDMemPage; // the memory page
typedef struct MerryDMemory MerryDMemory;   // the memory that manages these pages
typedef struct MerryDAddress MerryDAddress; // an internal struct

#define _MERRY_DMEMORY_PGALLOC_MAP_PAGE_ _MERRY_MEM_GET_PAGE_(_MERRY_MEMORY_ADDRESSES_PER_PAGE_, _MERRY_PROT_DEFAULT_, _MERRY_FLAG_DEFAULT_)
#define _MERRY_DMEMORY_PGALLOC_UNMAP_PAGE_(address) _MERRY_MEM_GIVE_PAGE_(address, _MERRY_MEMORY_ADDRESSES_PER_PAGE_)

#define _MERRY_DMEMORY_DEDUCE_ADDRESS_(addr)                                                                    \
    {                                                                                                           \
        .page = addr / _MERRY_MEMORY_ADDRESSES_PER_PAGE_, .offset = address % _MERRY_MEMORY_ADDRESSES_PER_PAGE_ \
    }

struct MerryDMemPage
{
    mbptr_t address_space;
    MerryMutex *lock;
};

struct MerryDMemory
{
    MerryDMemPage **pages;   // the pages
    msize_t number_of_pages; // the number of pages
    merrot_t error;          // any error that the Memory encounters
};

struct MerryDAddress
{
    unsigned int page;
    unsigned int offset;
};

MerryDMemory *merry_dmemory_init(msize_t num_of_pages);

MerryDMemory *merry_dmemory_init_provided(mqptr_t *mapped_pages, msize_t num_of_pages);

void merry_dmemory_free(MerryDMemory *memory);

mret_t merry_dmemory_read_byte(MerryDMemory *memory, maddress_t address, mbptr_t _store_in);

mret_t merry_dmemory_write_byte(MerryDMemory *memory, maddress_t address, mqword_t _to_write);

// Useless functions for now
mret_t merry_dmemory_read_lock(MerryDMemory *memory, maddress_t address, mqptr_t _store_in);
mret_t merry_dmemory_write_lock(MerryDMemory *memory, maddress_t address, mqword_t _to_write);
// -------

mptr_t merry_dmemory_get_address(MerryDMemory *memory, maddress_t address);

#endif