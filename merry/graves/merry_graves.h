#ifndef _MERRY_GRAVES_
#define _MERRY_GRAVES_

#include "merry_64_bit_core.h"
#include "merry_config.h"
#include "merry_consts.h"
#include "merry_core_base.h"
#include "merry_core_types.h"
#include "merry_dynamic_libs.h"
#include "merry_dynamic_list.h"
#include "merry_graves_constants.h"
#include "merry_graves_input.h"
#include "merry_graves_request_queue.h"
#include "merry_helpers.h"
#include "merry_list.h"
#include "merry_memory.h"
#include "merry_platform.h"
#include "merry_protectors.h"
#include "merry_ram.h"
#include "merry_state.h"
#include "merry_state_handlers.h"
#include "merry_threads.h"
#include "merry_types.h"
#include "merry_utils.h"
#include <stdatomic.h>

#define REQ_HDLR(name) void name(MerryGravesRequest *req)
#define PREQ_HDLR(name) void name(MerryGravesRequest *req)

typedef struct MerryGraves MerryGraves;
typedef struct MerryGravesCoreRepr MerryGravesCoreRepr;

struct MerryGravesCoreRepr {
  MerryCoreBase *base;
  void *cptr;
};

struct MerryGraves {
  mcoredetails_t core_base_func_list[__CORE_TYPE_COUNT];
  MerryDynamicList *all_cores;

  MerryGravesInput *reader;

  mcond_t master_cond;
  mmutex_t master_lock;

  MerryState master_state;

  MerryConsts sys_consts;

  msize_t core_count;
  msize_t lifetime_core_count;
  msize_t active_cores;

  mqword_t graves_return;
};

_MERRY_LOCAL_ MerryGraves graves;

// Once we have an input reader and input file format, Graves will receive the
// file path
mret_t merry_graves_init(int argc, char **argv);

mret_t merry_graves_acquaint_with_cores();

mret_t merry_graves_add_new_core(mcore_t c_type, maddress_t begin,
                                 mqword_t parent_id, mqword_t parent_uid,
                                 mbool_t priviledge, msize_t *id);

mret_t merry_graves_clean_a_core(msize_t cid);

mret_t merry_graves_find_old_core(msize_t *ind);

mptr_t merry_graves_get_hands_on_cptr(msize_t id);

mret_t merry_graves_bestow_priviledge(msize_t bestower, msize_t bestowed);

void merry_graves_encountered_error_serving(merrOrigin_t orig, mqword_t err,
                                            MerryCoreBase *base);

mbool_t merry_graves_check_vcore_priviledge_or_permission(msize_t id);

int merry_GRAVES_RULE(int argc, char **argv);

_THRET_T_ merry_graves_run_VM(void *arg);

void merry_graves_destroy();

REQ_HDLR(HANDLE_SHUTDOWN);
REQ_HDLR(HANDLE_LOADING_NEW_PAGE_DATA);
REQ_HDLR(HANDLE_LOADING_NEW_PAGE_INST);
REQ_HDLR(HANDLE_PROBLEM_ENCOUNTERED);
REQ_HDLR(HANDLE_PROGRAM_REQUEST);

/*--- Handling Program Requests ----*/
PREQ_HDLR(HANDLE_NEW_THREAD);
PREQ_HDLR(HANDLE_ADD_A_NEW_DATA_MEMORY_PAGE);
PREQ_HDLR(HANDLE_SAVE_STATE);
PREQ_HDLR(HANDLE_DELETE_STATE);
PREQ_HDLR(HANDLE_JMP_STATE);
PREQ_HDLR(HANDLE_SWITCH_STATE);
PREQ_HDLR(HANDLE_WILD_RESTORE);

#endif
