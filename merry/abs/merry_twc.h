#ifndef _MERRY_TWC_
#define _MERRY_TWC_

// Two-way channel
#include <merry_config.h>
#include <merry_platform.h>
#include <merry_types.h>
#include <merry_utils.h>
#include <merry_owc.h>
#include "merry_state.h"
#include <stdlib.h>

typedef struct MerryTWC MerryTWC;
typedef union MerryTWCBase MerryTWCBase; // This is converted to TWC after the child process is created

union MerryTWCBase
{
    struct
    {
        mdataline_t child[2];
        mdataline_t parent[2];
    } channels;
    struct
    {
        mdataline_t _read_line_for_child;
        mdataline_t _write_line_for_parent;
        mdataline_t _read_line_for_parent;
        mdataline_t _write_line_for_child;
    } lines;
};

struct MerryTWC
{
    MerryOWC rc;
    MerryOWC wc;
    MerryState state;
};

mret_t merry_twc_base_form(MerryTWCBase *base, MerryState *state);

// we create this using the base
MerryTWC *merry_twc_create(mdataline_t rline, mdataline_t wline, MerryState *state);

mret_t merry_twc_send(MerryTWC *twc, mbptr_t data, msize_t len);

mret_t merry_twc_receive(MerryTWC *twc, mbptr_t buf, msize_t n);

void merry_twc_destroy(MerryTWC *twc);

#endif
