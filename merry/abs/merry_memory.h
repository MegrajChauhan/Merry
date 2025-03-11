#ifndef _MERRY_MEMORY_
#define _MERRY_MEMORY_

#include <merry_config.h>
#include <merry_types.h>
#include <merry_utils.h>
#include <merry_consts.h>
#include <merry_platform.h>
#include <stdlib.h>

/**
 * This is not a custom allocator for the VM.
 * This will allocate memory pages from the host
 * The pages will be of constant size and we will introduce specific types of these pages too
 * With the ideas i have in my head, i cannot give up speed over the functionality they provide.
 * The complications that we might meet are too much to think about.
 */

typedef struct MerryMemoryPageBase MerryMemoryPageBase;
typedef MerryMemoryPageBase MerryNormalMemoryPage;

struct MerryMemoryPageBase
{
    mbptr_t buf; // The size of the buffer is known to us[a constant]
    mbool_t init;
};

#define merry_init_normal_memory_page(pg, buf)   \
    do                                           \
    {                                            \
        ((pg)->buf = (buf); (pg)->init = mtrue;) \
    } while (0)

#define merry_deinit_normal_memory_page(pg)                          \
    do                                                               \
    {                                                                \
        merry_check_ptr((pg));                                       \
        merry_check_ptr((pg)->buf);                                  \
        merry_return_memory_page((pg)->buf, _MERRY_BYTES_PER_PAGE_); \
    } while (0)

MerryNormalMemoryPage *merry_obtain_normal_memory_page();

void merry_return_normal_memory_page(MerryNormalMemoryPage *pg);

#endif