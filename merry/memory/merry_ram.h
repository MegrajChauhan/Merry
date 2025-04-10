#ifndef _MERRY_RAM_
#define _MERRY_RAM_

/**
 * The way we will define things here is:
 * 1) Either a core can have a private RAM that only they can use
 * 2) Or the core will share the same RAM with other cores
 * The tradeoff is that if the core has a private RAM than it is not allowed to access the shared
 * RAM. The shared RAM is managed by graves. We could have methods in place such as shared memory that
 * the core may use to communicate with the shared RAM
 */

#include <merry_utils.h>
#include <merry_types.h>
#include <merry_helpers.h>
#include <merry_memory.h>
#include <merry_protectors.h>
#include <merry_state.h>
#include <stdlib.h>

/**
 * The addresses are 64-bit and one page has _MERRY_PAGE_LEN_ addresses.
 * To any core or program, the RAM is going to be an array which is just linear with no breaks but the RAM will hide those details
 */

#define merry_deduce_address(addr, page_num, offset) \
    do                                               \
    {                                                \
        (page_num) = (addr) / _MERRY_PAGE_LEN_;      \
        (offset) = (addr) % _MERRY_PAGE_LEN_;        \
    } while (0)

typedef struct MerryRAM MerryRAM;
_MERRY_DEFINE_FUNC_PTR_(mret_t, mmem_byte_read_t, MerryRAM *, maddress_t, mbptr_t);
_MERRY_DEFINE_FUNC_PTR_(mret_t, mmem_word_read_t, MerryRAM *, maddress_t, mwptr_t);
_MERRY_DEFINE_FUNC_PTR_(mret_t, mmem_dword_read_t, MerryRAM *, maddress_t, mdptr_t);
_MERRY_DEFINE_FUNC_PTR_(mret_t, mmem_qword_read_t, MerryRAM *, maddress_t, mqptr_t);
_MERRY_DEFINE_FUNC_PTR_(mret_t, mmem_byte_write_t, MerryRAM *, maddress_t, mbyte_t);
_MERRY_DEFINE_FUNC_PTR_(mret_t, mmem_word_write_t, MerryRAM *, maddress_t, mword_t);
_MERRY_DEFINE_FUNC_PTR_(mret_t, mmem_dword_write_t, MerryRAM *, maddress_t, mdword_t);
_MERRY_DEFINE_FUNC_PTR_(mret_t, mmem_qword_write_t, MerryRAM *, maddress_t, mqptr_t);

struct MerryRAM
{
    MerryNormalMemoryPage **pages; // all of the pages
    msize_t page_count;            // Number of pages
    mmutex_t lock;
    MerryState state;
};

MerryRAM *merry_create_RAM(msize_t number_of_pages);

mret_t merry_RAM_add_pages(MerryRAM *ram, msize_t num); // add num new pages

mret_t merry_RAM_read_byte(MerryRAM *ram, maddress_t address, mbptr_t store_in);

mret_t merry_RAM_read_word(MerryRAM *ram, maddress_t address, mwptr_t store_in);

mret_t merry_RAM_read_dword(MerryRAM *ram, maddress_t address, mdptr_t store_in);

mret_t merry_RAM_read_qword(MerryRAM *ram, maddress_t address, mqptr_t store_in);

mret_t merry_RAM_write_byte(MerryRAM *ram, maddress_t address, mbyte_t value);

mret_t merry_RAM_write_word(MerryRAM *ram, maddress_t address, mword_t value);

mret_t merry_RAM_write_dword(MerryRAM *ram, maddress_t address, mdword_t value);

mret_t merry_RAM_write_qword(MerryRAM *ram, maddress_t address, mqword_t value);

void merry_destroy_RAM(MerryRAM *ram);

#endif