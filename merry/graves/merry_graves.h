#ifndef _MERRY_GRAVES_
#define _MERRY_GRAVES_

#include "merry_config.h"
#include "merry_types.h"
#include "merry_utils.h"
#include "merry_helpers.h"
#include "merry_consts.h"
#include "merry_platform.h"
#include "merry_list.h"
#include "merry_dynamic_list.h"
#include "merry_threads.h"
#include "merry_dynamic_libs.h"
#include "merry_state.h"
#include "merry_memory.h"
#include "merry_protectors.h"
#include "merry_ram.h"
#include "merry_graves_constants.h"
#include "merry_graves_request_queue.h"
#include "merry_core_base.h"
#include "merry_core_types.h"
#include "merry_64_bit_core.h"
#include "merry_graves_input.h"
#include "merry_state_handlers.h"
#include <stdatomic.h>

#define REQ_HDLR(name) void name(MerryGravesRequest *req)

typedef struct MerryGraves MerryGraves;
typedef struct MerryGravesCoreRepr MerryGravesCoreRepr;

struct MerryGravesCoreRepr
{
  MerryCoreBase *base;
  void *cptr;
};

struct MerryGraves
{
  MerryList *core_base_func_list;
  MerryDynamicList *all_cores;

  MerryGravesInput *reader;

  mcond_t master_cond;
  mmutex_t  master_lock;
  
  MerryState master_state;

  MerryConsts sys_consts;

  msize_t core_count;
  msize_t active_cores;

  mqword_t graves_return;
};

_MERRY_LOCAL_ MerryGraves graves;

// Once we have an input reader and input file format, Graves will receive the file path
mret_t merry_graves_init(int argc, char **argv);

mret_t merry_graves_acquaint_with_cores();

mret_t merry_graves_add_new_core(mcore_t c_type, maddress_t begin);

mret_t merry_graves_clean_a_core(msize_t cid);

mret_t merry_graves_find_old_core(msize_t *ind);

int merry_GRAVES_RULE(int argc, char **argv);

_THRET_T_ merry_graves_run_VM(void *arg);

void merry_graves_destroy();

REQ_HDLR(HANDLE_SHUTDOWN);
REQ_HDLR(HANDLE_LOADING_NEW_PAGE_DATA);
REQ_HDLR(HANDLE_LOADING_NEW_PAGE_INST);
REQ_HDLR(HANDLE_PROBLEM_ENCOUNTERED);
REQ_HDLR(HANDLE_PROGRAM_REQUEST);

#endif
