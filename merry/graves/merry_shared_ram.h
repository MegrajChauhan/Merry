#ifndef _MERRY_SHARED_RAM_
#define _MERRY_SHARED_RAM_

/*
 * There are three types of RAM:
 * 1) Public RAM
 * 2) Private RAM
 * 3) Shared RAM
 *    i) Privately Shared RAM
 *    ii) Globally Shared RAM
 *
 * Public RAM refer to the first RAM that are created when the input file is
 * read. Those RAM are as is and cannot be affected by any operations. Unlike
 * Public RAM, which are under the control of Graves, Private and Shared RAM are
 * under the control of the program.
 *
 * Private RAM are those RAM that a vcore may create but choose to keep it to
 * itself. Shared RAM, on the other hand, can be owned by multiple vcores. The
 * vcore may choose between the RAM it wants. It is to be noted that the
 * instruction memory cannot be changed or affected in any way. Private, Public
 * and Shared RAM are all different types of data RAM. One vcore may choose
 * between the RAM it wants.
 *
 * Privately Shared RAM are shared among just a few cores.
 * Globally Shared RAM are shared among all of the cores.
 *
 * This allows for a lot of possibilities.
 * */

#include "merry_dynamic_list.h"
#include "merry_graves_constants.h"
#include "merry_list.h"
#include "merry_ram.h"
#include "merry_state.h"
#include "merry_types.h"
#include "merry_utils.h"
#include <stdlib.h>

#define merry_is_RAM_shareable(ram) ((ram)->shareable)
#define merry_is_RAM_cross_group_shareable(ram) ((ram)->cross_group_shareable)
#define merry_is_RAM_id_valid(rid, rlist)                                      \
  ((rid) < merry_dynamic_list_size(rlist))
#define merry_is_RAM_size_fixed(ram) ((ram)->keep_fixed)
#define merry_is_RAM_limited(ram) ((ram)->have_max_size)
#define merry_get_RAM_shared_count(ram) ((ram)->shared_count)
#define merry_get_RAM_max_page_count(ram) ((ram)->max_page_count)
#define merry_get_RAM_state(ram) ((ram)->state)
#define merry_set_RAM_max_page_count(ram, mpcount)                             \
  ((ram)->max_page_count = (mpcount))
#define merry_make_RAM_shared(ram) ((ram)->shareable = mtrue)
#define merry_make_RAM_cross_group_shareable(ram)                              \
  ((ram)->cross_group_shareable = mtrue)
#define merry_make_RAM_fixed(ram) ((ram)->keep_fixed = mtrue)
#define merry_limit_RAM(ram) ((ram)->have_max_size = mtrue)

typedef struct MerryRAMRepr MerryRAMRepr;
typedef struct MerryRAMList MerryRAMList;
typedef enum mramstate_t mramstate_t;

enum mramstate_t {
  DEAD,  // The RAM is no longer used
  ALIVE, // The RAM is still in active use
};

struct MerryRAMRepr {
  MerryRAM *ram;
  msize_t ram_unique_id;
  msize_t shared_count; // How many vcores share this RAM?
  msize_t max_page_count;
  msize_t creator_core_id;
  msize_t creator_unique_id;
  msize_t creator_group_id;

  mbool_t shareable;             // Can this RAM be shared?
  mbool_t cross_group_shareable; // Should this RAM be shared between
                                 // groups(only if shareable is mtrue)?
  mbool_t keep_fixed;            // Keep the number of pages fixed
  mbool_t have_max_size; // Should the RAM have a limit to its dynamic length?

  mramstate_t state;
};

struct MerryRAMList {
  MerryDynamicList *rams;
  MerryList *all_dead_pages;
};

// Initially we do not have any RAM.
MerryRAMList *merry_create_RAM_list(MerryState *state);

mret_t merry_add_new_default_RAM(msize_t ccid, msize_t cuid, msize_t cgid,
                                 mqptr_t rid, msize_t pg_count,
                                 MerryState *state);

// l1 wants to share with l2
mret_t merry_share_RAM(msize_t rid, mqptr_t res, MerryRAMList *l1,
                       MerryRAMList *l2, MerryState *state);

MerryRAMRepr *merry_get_RAM(MerryRAMList *r1, msize_t rid);

msize_t merry_search_for_dead_RAM(MerryRAMList *r1);

mret_t merry_transfer_dead_RAM_pages(MerryRAMList *list, MerryState *state);

mret_t merry_revive_dead_RAM_pages(MerryRAMList *list, MerryRAMRepr *repr,
                                   MerryState *state);

void merry_kill_RAM(MerryRAMRepr *repr);

void merry_destroy_RAM_list(MerryRAMList *list);
// IMPLEMENT THIS SHIT!!!
#endif
