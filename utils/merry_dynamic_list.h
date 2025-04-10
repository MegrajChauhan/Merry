#ifndef _MERRY_DYNAMIC_LIST_
#define _MERRY_DYNAMIC_LIST_

/**
 * We will re-do the entire thing again!
 * If we abstract MerryList from this then we will have to bother with extra function calls which could add up
 */

#include <merry_list.h>

// Dynamic List will resize automatically if there is no extra storage
typedef MerryList MerryDynamicList; // The same base though

#define merry_is_dyn_list_empty(list) merry_is_list_empty(list)
#define merry_is_dyn_list_full(list) merry_is_list_full(list)
#define merry_dyn_list_has_at_least(list, len) merry_list_has_at_least(list, len)
#define merry_create_dynamic_list(capacity, elem_len) ((MerryDynamicList*)merry_create_list((capacity), (elem_len)))
#define merry_add_capacity_to_dynamic_list(list, _additional_cap) merry_add_capacity_to_list((MerryList*)(list), (_additional_cap))
#define merry_dynamic_list_resize(list, factor) merry_list_resize_list((MerryList*)(list), (factor))
#define merry_erase_dynamic_list(list) merry_erase_list((MerryList*)(list))
#define merry_destroy_dynamic_list(list) merry_destroy_list((MerryList*)(list))
#define merry_dynamic_list_pop(list) merry_list_pop((MerryList*)(list))

// The extra conversions are probably unnecessary given that MerryDynamicList is just a sugar coating around MerryList in which the
// push and pop behave slightly differently

mret_t merry_dynamic_list_push(MerryDynamicList *list, mptr_t elem);

#endif