#ifndef _MERRY_DMEMORY_
#define _MERRY_DMEMORY_

#include <merry_internals.h>
#include <merry_errors.h>
#include <merry_thread.h>
#include <merry_mem.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include "merry_request.h"
#include "merry_console.h"

typedef struct MerryDMemPage MerryDMemPage; // the memory page
typedef struct MerryDMemory MerryDMemory;   // the memory that manages these pages
typedef struct MerryDAddress MerryDAddress; // an internal struct

#define _MERRY_DMEMORY_PGALLOC_MAP_PAGE_ _MERRY_MEM_GET_PAGE_(_MERRY_MEMORY_ADDRESSES_PER_PAGE_, _MERRY_PROT_DEFAULT_, _MERRY_FLAG_DEFAULT_)
#define _MERRY_DMEMORY_PGALLOC_UNMAP_PAGE_(address) _MERRY_MEM_GIVE_PAGE_(address, _MERRY_MEMORY_ADDRESSES_PER_PAGE_)

#define _MERRY_DMEMORY_DEDUCE_ADDRESS_(addr) \
    {                                        \
        .page = addr / _MERRY_MEMORY_ADDRESSES_PER_PAGE_, .offset = address % _MERRY_MEMORY_ADDRESSES_PER_PAGE_}

struct MerryDMemPage
{
    mbptr_t address_space;
};

struct MerryDMemory
{
    MerryDMemPage **pages;   // the pages
    msize_t number_of_pages; // the number of pages
    merrot_t error;          // any error that the Memory encounters
    MerryCond *cond;
    MerryMutex *lock;
};

struct MerryDAddress
{
    unsigned int page;
    unsigned int offset;
};

extern mret_t merry_requestHdlr_push_request(msize_t req_id, msize_t id, MerryCond *req_cond);

MerryDMemory *merry_dmemory_init(msize_t num_of_pages);

MerryDMemory *merry_dmemory_init_provided(mbptr_t *mapped_pages, msize_t num_of_pages);

mret_t merry_dmemory_add_new_page(MerryDMemory *memory);

void merry_dmemory_free(MerryDMemory *memory);

mret_t merry_dmemory_read_byte(MerryDMemory *memory, maddress_t address, mqptr_t _store_in);
mret_t merry_dmemory_read_byte_atm(MerryDMemory *memory, maddress_t address, mqptr_t _store_in);
mret_t merry_dmemory_write_byte(MerryDMemory *memory, maddress_t address, mqword_t _to_write);
mret_t merry_dmemory_write_byte_atm(MerryDMemory *memory, maddress_t address, mqword_t _to_write);

mret_t merry_dmemory_read_word(MerryDMemory *memory, maddress_t address, mqptr_t _store_in);
mret_t merry_dmemory_read_word_atm(MerryDMemory *memory, maddress_t address, mqptr_t _store_in);
mret_t merry_dmemory_write_word(MerryDMemory *memory, maddress_t address, mqword_t _to_write);
mret_t merry_dmemory_write_word_atm(MerryDMemory *memory, maddress_t address, mqword_t _to_write);

mret_t merry_dmemory_read_dword(MerryDMemory *memory, maddress_t address, mqptr_t _store_in);
mret_t merry_dmemory_read_dword_atm(MerryDMemory *memory, maddress_t address, mqptr_t _store_in);
mret_t merry_dmemory_write_dword(MerryDMemory *memory, maddress_t address, mqword_t _to_write);
mret_t merry_dmemory_write_dword_atm(MerryDMemory *memory, maddress_t address, mqword_t _to_write);

mret_t merry_dmemory_read_qword(MerryDMemory *memory, maddress_t address, mqptr_t _store_in);
mret_t merry_dmemory_read_qword_atm(MerryDMemory *memory, maddress_t address, mqptr_t _store_in);
mret_t merry_dmemory_write_qword(MerryDMemory *memory, maddress_t address, mqword_t _to_write);
mret_t merry_dmemory_write_qword_atm(MerryDMemory *memory, maddress_t address, mqword_t _to_write);

mbptr_t merry_dmemory_get_byte_address(MerryDMemory *memory, maddress_t address);
mbptr_t merry_dmemory_get_byte_address_bounds(MerryDMemory *memory, maddress_t address, msize_t bound);

mwptr_t merry_dmemory_get_word_address(MerryDMemory *memory, maddress_t address);
mwptr_t merry_dmemory_get_word_address_bounds(MerryDMemory *memory, maddress_t address, msize_t bound);

mdptr_t merry_dmemory_get_dword_address(MerryDMemory *memory, maddress_t address);
mdptr_t merry_dmemory_get_dword_address_bounds(MerryDMemory *memory, maddress_t address, msize_t bound);

mqptr_t merry_dmemory_get_qword_address(MerryDMemory *memory, maddress_t address);
mqptr_t merry_dmemory_get_qword_address_bounds(MerryDMemory *memory, maddress_t address, msize_t bound);

// Specially added functions for making the job of the VCore easier

/*
 This function will return an array of bytes of length 'length'
 It will make the task for the core easier by allowing for strings to cover multiple pages
 The returned pointer will be freed immediately
*/
mstr_t merry_dmemory_get_bytes_maybe_over_multiple_pages(MerryDMemory *memory, maddress_t address, msize_t length);
mstr_t merry_dmemory_get_bytes_maybe_over_multiple_pages_upto(MerryDMemory *memory, maddress_t address, char byte);

mret_t merry_dmemory_write_bytes_maybe_over_multiple_pages(MerryDMemory *memory, maddress_t address, msize_t length, mbptr_t array);

void merry_dmemory_read_from_next_page(MerryDMemory *mem, msize_t _nxt_pg, msize_t arr_len, msize_t arr_offset, mbptr_t arr);
void merry_dmemory_write_to_next_page(MerryDMemory *mem, msize_t _nxt_pg, mqword_t _to_write, msize_t _from, msize_t _to);
void merry_dmemory_read_from_next_page_atm(MerryDMemory *mem, msize_t _nxt_pg, msize_t arr_len, msize_t arr_offset, mbptr_t arr);
void merry_dmemory_write_to_next_page_atm(MerryDMemory *mem, msize_t _nxt_pg, mqword_t _to_write, msize_t _from, msize_t _to);

#endif