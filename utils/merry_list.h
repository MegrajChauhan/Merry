#ifndef _MERRY_LIST_
#define _MERRY_LIST_

#include "merry_state.h"
#include <merry_config.h>
#include <merry_types.h>
#include <merry_utils.h>
#include <stdlib.h>
#include <string.h> // memcpy

typedef struct MerryList MerryList;

struct MerryList {
  mptr_t buf;
  msize_t elem_len;
  msize_t buf_cap;
  msize_t curr_ptr; // the current index when there is an element
  msize_t max_ind;
  MerryState lstate;
};

// curr_ptr points to the last element that was pushed

// These macro functions are a risk in the sense that we cannot check for the
// validity of the pointers so we must depend on the caller that the pointers
// are valid
#define merry_is_list_empty(list) ((list)->curr_ptr == (msize_t)(-1))
#define merry_is_list_full(list)                                               \
  (!merry_is_list_empty(list) && (list)->curr_ptr >= (list)->max_ind)
#define merry_list_has_at_least(list, len)                                     \
  (!(merry_is_list_empty(list)) &&                                             \
   ((((list)->curr_ptr) < (list)->max_ind) && (list)->curr_ptr + 1) >= (len))
#define merry_list_size(list) ((list) ? (list)->curr_ptr : -1)

MerryList *merry_create_list(msize_t capacity, msize_t elem_len,
                             MerryState *state);

// MerryList is not dynamic by default and hence we need ways to increase the
// estimated size This adds to the current capacity
mret_t merry_add_capacity_to_list(MerryList *list, msize_t _additional_cap);

// Here, factor represents the number of times the current capacity is to be
// multiplied
mret_t merry_list_resize_list(MerryList *list, msize_t factor);

mret_t merry_list_push(MerryList *list, mptr_t elem);

mptr_t merry_list_pop(MerryList *list);

mptr_t merry_list_at(MerryList *list, msize_t at);

mret_t merry_list_replace(MerryList *list, mptr_t new_elem, msize_t at);

// we won't be providing iterators given how it is not safe
void merry_erase_list(MerryList *list);

void merry_destroy_list(MerryList *list);

#endif
