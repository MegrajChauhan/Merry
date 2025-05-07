#ifndef _MERRY_MEMORY_
#define _MERRY_MEMORY_

#include <merry_config.h>
#include <merry_types.h>
#include <merry_utils.h>
#include <merry_consts.h>
#include <merry_platform.h>
#include <merry_state.h>
#include <stdlib.h>
#include <string.h>

/**
 * This is not a custom allocator for the VM.
 * This will allocate memory pages from the host
 * The pages will be of constant size and we will introduce specific types of these pages too
 * With the ideas i have in my head, i cannot give up speed over the functionality they provide.
 * The complications that we might meet are too much to think about.
 */

_MERRY_INTERNAL_ MerryState memory_state; // This state dictates the state of VM's interface to the OS for memory

typedef struct MerryMemoryPageBase MerryMemoryPageBase;
typedef MerryMemoryPageBase MerryNormalMemoryPage;

struct MerryMemoryPageBase
{
    mbptr_t buf; // The size of the buffer is known to us[a constant]
    mbool_t init;
};

void merry_initialize_memory_interface();

void merry_obtain_memory_interface_state(MerryState *st);

MerryNormalMemoryPage *merry_create_normal_memory_page();

mret_t merry_initialize_normal_memory_page(MerryNormalMemoryPage *pg);

MerryNormalMemoryPage *merry_obtain_normal_memory_page();

void merry_return_normal_memory_page(MerryNormalMemoryPage *pg);

#endif
