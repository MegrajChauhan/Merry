#include "merry_shared_ram.h"

MerryRAMList *merry_create_RAM_list(MerryState *state) {

  MerryRAMList *list = (MerryRAMList *)malloc(sizeof(MerryRAMList));
  if (!list) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    return RET_NULL;
  }
  if ((list->rams = merry_create_dynamic_list(1, sizeof(MerryRAMRepr *),
                                              state)) == RET_NULL) {
    free(list);
    return RET_NULL;
  }

  if ((list->all_dead_pages =
           merry_create_list(_MERRY_DEAD_PAGES_BUFFER_LEN_,
                             sizeof(MerryRAMRepr *), state)) == RET_NULL) {
    merry_destroy_dynamic_list(list->rams);
    free(list);
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

MerryRAMRepr *merry_get_RAM(MerryRAMList *r1, msize_t rid) {
  merry_check_ptr(r1);
  merry_check_ptr(r1->all_dead_pages);
  merry_check_ptr(r1->rams);

  if (!merry_is_RAM_id_valid(rid, r1->rams))
    return RET_NULL;

  return *((MerryRAMRepr **)merry_dynamic_list_at(r1->rams, rid));
}

msize_t merry_search_for_dead_RAM(MerryRAMList *r1) {
  merry_check_ptr(r1);
  merry_check_ptr(r1->all_dead_pages);
  merry_check_ptr(r1->rams);

  for (msize_t i = 0; i < merry_dynamic_list_size(r1->rams); i++) {
    MerryRAMRepr *repr =
        (((MerryRAMRepr **)
              r1->rams->buf)[i]); // direct access for faster execution
    if (repr->state == DEAD)
      return i;
  }
  return (mqword_t)(-1);
}

mret_t merry_reclaim_dead_RAM_pages(MerryRAMList *list, MerryState *state);

mret_t merry_revive_dead_RAM_pages(MerryRAMList *list, MerryRAMRepr *repr,
                                   msize_t num_of_pages, MerryState *state) {
  // Here we have a new RAM that was either previously dead
  // or is new. No matter the case, we have to populate the new
  // RAM with dead pages.
  merry_check_ptr(list);
  merry_check_ptr(repr);
  merry_check_ptr(list->rams);
  merry_check_ptr(list->all_dead_pages);

  if (surelyF(num_of_pages == 0))
    merry_unreachable("Reviving %zu pages with 0 dead pages", num_of_pages);

  if (merry_is_list_empty(list->all_dead_pages))
    merry_unreachable("Reviving pages that don't exist{Requested: %zu}",
                      num_of_pages);
  // TODO: Add unreachable code sections everywhere in the codebase.

  msize_t pages_to_be_revived =
      (num_of_pages > merry_list_size(list->all_dead_pages)
           ? merry_list_size(list->all_dead_pages)
           : num_of_pages);

  // we have pages_to_be_revived number of pages to revive.
  // Go through each page and revive it one by one.
  // We can do it all at once too but I am leaving some
  // place for flexibility if needed
  for (msize_t i = 0; i < pages_to_be_revived; i++) {
  }

  return RET_SUCCESS;
}

mret_t merry_kill_RAM(MerryRAMList *list, MerryRAMRepr *repr,
                      MerryState *state) {
  // we will not free repr but declare it dead.
  // The RAM will not be free'd either but it will be
  // stripped off of its pages
  merry_check_ptr(repr);
  merry_check_ptr(repr->ram);

  if (repr->state == DEAD)
    return RET_SUCCESS; // already dead

  if (merry_reclaim_dead_RAM_pages(list, state) == RET_FAILURE)
    return RET_FAILURE;

  repr->state = DEAD;
  // every other properties is reset when reused
  return RET_SUCCESS;
}

void merry_destroy_RAM_list(MerryRAMList *list) {
  merry_check_ptr(list);
  merry_check_ptr(list->all_dead_pages);
  merry_check_ptr(list->rams);

  for (msize_t i = 0; i < merry_dynamic_list_size(list->rams); i++) {
    MerryRAMRepr *repr =
        (((MerryRAMRepr **)
              list->rams->buf)[i]); // direct access for faster execution
    merry_destroy_RAM(repr->ram);
    free(repr);

    /*
     * Why is this safe?
     * When a core shares its RAM with some other core and if either
     * of them free'd the RAM first then the other will trigger a
     * double free exception which is why, although it takes more
     * memory, we will allow Graves to keep track of every RAM list
     * created and Graves will be entrusted with cleaning the
     * RAMLists.
     * */
  }

  for (msize_t i = 0; i < merry_list_size(list->all_dead_pages); i++) {
    MerryNormalMemoryPage *page =
        (((MerryNormalMemoryPage **)list->all_dead_pages->buf)[i]);
    merry_return_normal_memory_page(page);
  }

  merry_destroy_dynamic_list(list->rams);
  merry_destroy_list(list->all_dead_pages);
  free(list);
}
