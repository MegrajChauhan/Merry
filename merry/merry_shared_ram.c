#include "merry_shared_ram.h"

MerryRAMList *merry_create_RAM_list(MerryState *state) {

  MerryRAMList *list = (MerryRAMList *)malloc(sizeof(MerryRAMList));
  if (!list) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    return RET_NULL;
  }
  return list;
}

mret_t merry_add_new_default_RAM(msize_t ccid, msize_t cuid, msize_t cgid,
                                 mqptr_t rid, msize_t pg_count,
                                 MerryState *state) {
  // create a new RAM that is default and useless for now
  // OR we can just reuse an existing one

  return RET_SUCCESS;
}

// l1 wants to share with l2
mret_t merry_share_RAM(msize_t rid, mqptr_t res, MerryRAMList *l1,
                       MerryRAMList *l2, MerryState *state);

mret_t merry_move_pages_from_dead_RAM_to_alive_RAM(MerryRAMRepr *alive,
                                                   MerryRAMRepr *dead,
                                                   msize_t count,
                                                   MerryState *state);

MerryRAMRepr *merry_get_RAM(MerryRAMList *r1, msize_t rid);

msize_t merry_search_for_dead_RAM(MerryRAMList *r1, msize_t threshold);

void merry_kill_RAM(MerryRAMRepr *repr);

void merry_keep_RAM_fixed(MerryRAMRepr *repr);

void merry_destroy_RAM_list(MerryRAMList *list);
