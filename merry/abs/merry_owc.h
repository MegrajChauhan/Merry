#ifndef _MERRY_OWC_
#define _MERRY_OWC_

// One-way channel
#include <merry_config.h>
#include <merry_platform.h>
#include <merry_types.h>
#include <merry_utils.h>
#include <stdlib.h>

typedef struct MerryOWC MerryOWC;

/**
 * When two processes are communicating via MerryOWC, a copy is given to each of the process.
 * One process should only listen by closing it's write line while the other should only speak
 * by closing the read line.
 * If both processes need to speak and listen, two OWC can be created.
 * Or, in our case, we will create a TWC or Two-way channel
 */
struct MerryOWC
{
    union
    {
        mdataline_t pfd[2]; // the file descriptors
        struct
        {
            mdataline_t _read_line;
            mdataline_t _write_line;
        };
    };
    mbool_t _in_use;
    mbool_t _rclosed;
    mbool_t _wclosed;
};

#define merry_owc_init(owc, rline, wline, rclosed, wclosed) \
    do                                                      \
    {                                                       \
        (owc)->_read_line = (rline);                        \
        (owc)->_write_line = (wline);                       \
        (owc)->_in_use = mtrue;                             \
        (owc)->_rclosed = (rclosed);                        \
        (owc)->_wclosed = (wclosed);                        \
    } while (0)

MerryOWC *merry_open_owc();

void merry_owc_only_speak(MerryOWC *owc);

void merry_owc_only_listen(MerryOWC *owc);

void merry_owc_free_channel(MerryOWC *owc);

mret_t merry_owc_reopen(MerryOWC *owc);

void merry_destroy_owc(MerryOWC *owc);

mret_t merry_owc_speak(MerryOWC *owc, mbptr_t data, msize_t len);

mret_t merry_owc_listen(MerryOWC *owc, mbptr_t buf, msize_t n);

#endif