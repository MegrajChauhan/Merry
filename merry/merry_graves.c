#include "merry_graves.h"

mret_t merry_graves_init(int argc, char **argv) {
  merry_check_ptr(argv);

  if (merry_parse_arg(argc, argv, &graves.sys_consts) == RET_FAILURE) {
    merry_msg("Cannot START MERRY....", NULL);
    return RET_FAILURE;
  }
  merry_provide_context(graves.master_state, _MERRY_GRAVES_INITIALIZATION_);

  merry_initialize_memory_interface();

  if ((graves.reader = merry_graves_initialize_reader(
           argv[graves.sys_consts.inp_file_index], &graves.master_state)) ==
      RET_NULL) {
    merry_MAKE_SENSE_OF_STATE(&graves.master_state);
    return RET_FAILURE;
  }

  if (merry_graves_reader_read_input(graves.reader) == RET_FAILURE) {
    merry_provide_context(graves.master_state, _MERRY_GRAVES_INITIALIZATION_);
    graves.master_state.child_state = &graves.reader->state;
    merry_MAKE_SENSE_OF_STATE(&graves.master_state);
    return RET_FAILURE;
  }

  if ((graves.all_cores = merry_create_dynamic_list(
           __CORE_TYPE_COUNT, sizeof(MerryGravesCoreRepr),
           &graves.master_state)) == RET_NULL) {
    merry_MAKE_SENSE_OF_STATE(&graves.master_state);
    merry_graves_destroy();
    return RET_FAILURE;
  }

  if (merry_graves_acquaint_with_cores() == RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&graves.master_state);
    merry_graves_destroy();
    return RET_FAILURE;
  }

  if (merry_cond_init(&graves.master_cond) == RET_FAILURE) {
    merry_assign_state(graves.master_state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FAILED_TO_OBTAIN_COND_);
    merry_MAKE_SENSE_OF_STATE(&graves.master_state);
    merry_graves_destroy();
    return RET_FAILURE;
  }

  if (merry_mutex_init(&graves.master_lock) == RET_FAILURE) {
    merry_assign_state(graves.master_state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FAILED_TO_OBTAIN_LOCK_);
    merry_MAKE_SENSE_OF_STATE(&graves.master_state);
    merry_graves_destroy();
    return RET_FAILURE;
  }

  if (merry_graves_req_queue_init(&graves.master_cond, &graves.master_state) ==
      RET_FAILURE) {
    merry_provide_context(graves.master_state, _MERRY_GRAVES_INITIALIZATION_);
    merry_MAKE_SENSE_OF_STATE(&graves.master_state);
    merry_graves_destroy();
    return RET_FAILURE;
  }

  // We need to pass the program arguments somehow.
  // We shall do it here.
  graves.core_count = 0;
  graves.lifetime_core_count = 0;
  graves.group_count = 0;

  return RET_SUCCESS;
}

mret_t merry_graves_acquaint_with_cores() {
  graves.core_base_func_list[0] = merry_64_bit_core_base;
  return RET_SUCCESS;
}

mret_t merry_graves_find_old_core(msize_t *ind) {
  if (graves.active_cores == graves.core_count)
    return RET_FAILURE;
  for (msize_t i = 0; i < graves.core_count; i++) {
    MerryGravesCoreRepr *repr =
        merry_dynamic_list_at(graves.all_cores, i); // this shouldn't fail
    if (repr->cptr == NULL) {
      *ind = i;
      return RET_SUCCESS;
    }
  }
  return RET_FAILURE;
}

mret_t merry_graves_add_new_core(mcore_t c_type, maddress_t begin,
                                 mqword_t parent_id, mqword_t parent_uid,
                                 mqword_t parent_group, mbool_t priviledge,
                                 msize_t *id) {
  MerryCoreBase *base;
  mcoredetails_t details = graves.core_base_func_list[c_type];

  if ((base = (details)(&graves.master_state)) == RET_NULL)
    return RET_FAILURE;

  void *tmp;
  if ((tmp = base->init_func(base, graves.reader->data_ram,
                             graves.reader->iram[c_type], begin)) == RET_NULL) {
    graves.master_state = base->state;
    merry_core_base_clean(base);
    return RET_FAILURE;
  }
  msize_t i;
  base->priviledge = priviledge;
  if (merry_graves_find_old_core(&i) == RET_FAILURE) {
    MerryGravesCoreRepr repr;
    repr.base = base;
    repr.cptr = tmp;
    base->core_id = graves.core_count;

    if (merry_dynamic_list_push(graves.all_cores, &repr) == RET_FAILURE) {
      merry_assign_state(graves.master_state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_FAILED_TO_ADD_CORE_);
      graves.master_state.child_state = &graves.all_cores->lstate;
      merry_core_base_clean(base);
      return RET_FAILURE;
    }
    if (id)
      *id = base->core_id;
    graves.core_count++;
  } else {
    MerryGravesCoreRepr *repr = merry_dynamic_list_at(graves.all_cores, i);
    repr->base = base;
    repr->cptr = tmp;
    base->core_id = i;
    if (id)
      *id = base->core_id;
  }
  base->unique_id = graves.lifetime_core_count;
  graves.lifetime_core_count++;
  base->parent_core_id = parent_id;
  base->parent_unique_id = parent_uid;
  base->group_id = parent_group;
  return RET_SUCCESS;
}

mret_t merry_graves_bestow_priviledge(MerryGravesCoreRepr *bestower,
                                      MerryGravesCoreRepr *bestowed,
                                      mqptr_t ret) {
  merry_check_ptr(bestower);
  merry_check_ptr(bestowed);
  // if bestowed is invalid, we fail silently
  if (bestowed->base->group_id != bestower->base->group_id) {
    *ret = NOT_THE_SAME_GROUP;
    return RET_FAILURE;
  } else if (bestower->base->priviledge != mtrue) {
    *ret = NOT_PRIVILEDGED;
    return RET_FAILURE;
  }
  atomic_store((atomic_bool *)&bestowed->base->priviledge, mtrue);
  return RET_SUCCESS;
}

void merry_graves_encountered_error_serving(merrOrigin_t orig, mqword_t err,
                                            MerryCoreBase *base) {
  merry_assign_state(base->state, orig, err);
  merry_provide_context(base->state, _MERRY_GRAVES_SERVING_REQUEST_);
  base->state.arg.qword = base->core_id;
  merry_MAKE_SENSE_OF_STATE(&base->state);
  base->terminate = mtrue;
  base->stop = mtrue;
}

mret_t merry_graves_boot_core(msize_t core_id) {
  MerryGravesCoreRepr *repr;

  if ((repr = merry_dynamic_list_at(graves.all_cores, core_id)) == RET_NULL)
    return RET_FAILURE; // This will never happen

  mthread_t th; // We won't need this later unless we are creating an attached
                // thread which we are not

  if (merry_create_detached_thread(&th, repr->base->exec_func, repr->cptr,
                                   &graves.master_state) == RET_FAILURE) {
    return RET_FAILURE;
  }

  return RET_SUCCESS;
}

mret_t merry_graves_clean_a_core(msize_t cid) {
  MerryGravesCoreRepr *repr;
  if ((repr = merry_dynamic_list_at(graves.all_cores, cid)) == RET_NULL) {
    // This will never execute since cid is valid
    merry_log("The hell!! At cleaning a core.\n", NULL);
    return RET_FAILURE;
  }
  merry_cond_signal(&repr->base->cond); // tell the core to continue
  repr->base->free_func(repr->cptr);    // the base is cleaned up as well
  repr->cptr = NULL;
  repr->base = NULL;
  graves.active_cores--;
  return RET_SUCCESS;
}

mptr_t merry_graves_get_hands_on_cptr(msize_t id) {
  return ((MerryGravesCoreRepr *)merry_dynamic_list_at(graves.all_cores, id))
      ->cptr; // id is always valid
}

_THRET_T_ merry_graves_run_VM(void *arg) {

  // First start a new core
  if (merry_graves_add_new_core(graves.reader->itit.entries[0].type, 0, -1, -1,
                                0, mtrue, NULL) == RET_FAILURE) {
    merry_provide_context(graves.master_state, _MERRY_GRAVES_INITIALIZATION_);
    merry_MAKE_SENSE_OF_STATE(&graves.master_state);
    return NULL;
  }
  if (merry_graves_boot_core(0) == RET_FAILURE) {
    merry_provide_context(graves.master_state, _MERRY_GRAVES_BOOTING_A_CORE_);
    graves.master_state.arg.qword = 0;
    merry_MAKE_SENSE_OF_STATE(&graves.master_state);
    return NULL;
  }
  graves.active_cores++;

  MerryGravesRequest *request;

  while (graves.active_cores != 0) {
    if (merry_graves_wants_work(&request) == RET_FAILURE) {
      merry_cond_wait(&graves.master_cond, &graves.master_lock);
    } else {
      switch (request->type) {
      case SHUT_DOWN:
        HANDLE_SHUTDOWN(request);
        break;
      case TRY_LOADING_NEW_PAGE_DATA:
        HANDLE_LOADING_NEW_PAGE_DATA(request);
        merry_cond_signal(&request->base->cond);
        break;
      case TRY_LOADING_NEW_PAGE_INST:
        HANDLE_LOADING_NEW_PAGE_INST(request);
        merry_cond_signal(&request->base->cond);
        break;
      case PROBLEM_ENCOUNTERED:
        HANDLE_PROBLEM_ENCOUNTERED(request);
        merry_cond_signal(&request->base->cond);
        break;
      case PROGRAM_REQUEST:
        HANDLE_PROGRAM_REQUEST(request);
        merry_cond_signal(&request->base->cond);
        break;
      }
    }
  }

  return (_THRET_T_)NULL;
}

int merry_GRAVES_RULE(int argc, char **argv) {
  MerryState global;
  if (merry_graves_init(argc, argv) == RET_FAILURE)
    return 1;
  mthread_t th;
  if (merry_thread_create(&th, merry_graves_run_VM, NULL, &global) ==
      RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&global);
    return 1;
  }
  merry_thread_join(th, NULL);
  merry_graves_destroy();
  return 0;
}

void merry_graves_destroy() {
  if (!graves.all_cores) {
    for (msize_t i = 0; i <= graves.core_count; i++) {
      MerryGravesCoreRepr *repr = merry_dynamic_list_pop(graves.all_cores);
      if (repr->cptr != NULL)
        repr->base->free_func(repr->cptr);
    }
    merry_destroy_dynamic_list(graves.all_cores);
  }
  merry_graves_reader_destroy(graves.reader);
  merry_cond_destroy(&graves.master_cond);
  merry_mutex_destroy(&graves.master_lock);
  merry_graves_req_queue_free();
}

REQ_HDLR(HANDLE_SHUTDOWN) { merry_graves_clean_a_core(req->base->core_id); }

REQ_HDLR(HANDLE_LOADING_NEW_PAGE_INST) {
  if (merry_graves_reader_load_instructions(graves.reader, req->base->core_type,
                                            req->args[0] / _MERRY_PAGE_LEN_) ==
      RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&graves.reader->state);
    req->args[0] = REQUEST_FAILED;
  } else {
    req->args[0] = REQUEST_SERVED;
  }
}

REQ_HDLR(HANDLE_LOADING_NEW_PAGE_DATA) {
  if (merry_graves_reader_load_data(
          graves.reader, req->args[0] / _MERRY_PAGE_LEN_) == RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&graves.reader->state);
    req->args[0] = REQUEST_FAILED;
  } else {
    req->args[0] = REQUEST_SERVED;
  }
}

REQ_HDLR(HANDLE_PROBLEM_ENCOUNTERED) {
  merry_MAKE_SENSE_OF_STATE(&req->base->state);
}

REQ_HDLR(HANDLE_PROGRAM_REQUEST) {
  switch (req->args[0]) {
  case NEW_THREAD:
    HANDLE_NEW_THREAD(req);
    break;
  case ADD_A_NEW_DATA_MEMORY_PAGE:
    HANDLE_ADD_A_NEW_DATA_MEMORY_PAGE(req);
    break;
  case NEW_GROUP:
    HANDLE_NEW_GROUP(req);
    break;
  case SAVE_STATE:
    HANDLE_SAVE_STATE(req);
    break;
  case DELETE_STATE:
    HANDLE_DELETE_STATE(req);
    break;
  case JMP_STATE:
    HANDLE_JMP_STATE(req);
    break;
  case SWITCH_STATE:
    HANDLE_SWITCH_STATE(req);
    break;
  case WILD_RESTORE:
    HANDLE_WILD_RESTORE(req);
    break;
  case BESTOW_PRIVILEDGE:
    HANDLE_BESTOW_PRIVILEDGE(req);
    break;
  case DO_NOT_DISTURB:
    HANDLE_DO_NOT_DISTURB(req);
    break;
  case IGNORE_PAUSE:
    HANDLE_IGNORE_PAUSE(req);
    break;
  case WILD_REQUEST:
    HANDLE_WILD_REQUEST(req);
    break;
  case PAUSE:
    HANDLE_PAUSE(req);
    break;
  case UNPAUSE:
    HANDLE_UNPAUSE(req);
    break;
  case GRANT_PERMISSION:
    HANDLE_GRANT_PERMISSION(req);
    break;
  case IS_CORE_DEAD:
    HANDLE_IS_CORE_DEAD(req);
    break;
  case IS_PARENT_ALIVE:
    HANDLE_IS_PARENT_ALIVE(req);
    break;
  case GET_CID:
    HANDLE_GET_CID(req);
    break;
  case GET_UID:
    HANDLE_GET_UID(req);
    break;
  case GET_GROUP:
    HANDLE_GET_GROUP(req);
    break;
  case KILL_CORE:
    HANDLE_KILL_CORE(req);
    break;
  case IS_CHILD:
    HANDLE_IS_CHILD(req);
    break;
  case CHANGE_PARENT:
    HANDLE_CHANGE_PARENT(req);
    break;
  case CHANGE_CHILD_PARENT:
    HANDLE_CHANGE_CHILD_PARENT(req);
    break;
  case GIVEUP_PRIVILEDGE:
    HANDLE_GIVEUP_PRIVILEDGE(req);
    break;
  case PURGE_WREQUESTS:
    HANDLE_PURGE_WREQUESTS(req);
    break;
  default:
    merry_log("INVALREQ: Invalid request ID[%zu][CID: %zu{UID: %zu}]",
              req->args[0], req->base->core_id, req->base->unique_id);
    break;
  }
}

/*------Handling Program Requests------*/

PREQ_HDLR(HANDLE_NEW_THREAD) {
  MerryGravesCoreRepr *repr =
      merry_dynamic_list_at(graves.all_cores, req->base->core_id);
  if (!merry_graves_check_vcore_priviledge_or_permission(repr)) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_PRIVILEDGED;
    return;
  }
  msize_t id = 0;
  if (merry_graves_add_new_core(req->args[1] & __CORE_TYPE_COUNT, req->args[2],
                                req->base->core_id, req->base->unique_id,
                                req->base->group_id, req->args[3] & 0xFF,
                                &id) == RET_FAILURE) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = FAILED_TO_ADD_CORE;
    return;
  }
  if (merry_graves_boot_core(id) == RET_FAILURE) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = FAILED_TO_ADD_CORE;
    return;
  }
  req->args[0] = REQUEST_SERVED;
  req->args[1] = id;
}

PREQ_HDLR(HANDLE_ADD_A_NEW_DATA_MEMORY_PAGE) {
  MerryGravesCoreRepr *repr =
      merry_dynamic_list_at(graves.all_cores, req->base->core_id);
  if (!merry_graves_check_vcore_priviledge_or_permission(repr)) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_PRIVILEDGED;
    return;
  }
  msize_t tmp = req->base->ram->page_count;

  if (merry_RAM_add_pages(req->base->ram, req->args[1], &graves.master_state) ==
      RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&graves.master_state);
    req->args[0] = REQUEST_FAILED;
    req->args[1] = FAILED_TO_ADD_DATA_MEMORY_PAGE;
  } else {
    req->args[0] = REQUEST_SERVED;
    req->args[1] = tmp * _MERRY_BYTES_PER_PAGE_;
  }
}

PREQ_HDLR(HANDLE_SAVE_STATE) {
  if (req->base->wrequest == mtrue) {
    merry_graves_encountered_error_serving(
        _MERRY_PROGRAM_ERROR_,
        _MERRY_ATTEMPT_TO_PERFORM_STATE_OPERATIONS_DURING_WREQUEST_HANDLING,
        req->base);
    return;
  }
  void *cptr = merry_graves_get_hands_on_cptr(req->base->core_id);
  if (req->base->save_state_func(cptr) == RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&req->base->state);
    req->args[0] = REQUEST_FAILED;
    req->args[1] = FAILED_STATE_OPERATION;
    return;
  }
  req->args[0] = REQUEST_SERVED;
  req->args[1] = merry_dynamic_list_size(req->base->execution_states);
}

PREQ_HDLR(HANDLE_DELETE_STATE) {
  if (req->base->wrequest == mtrue) {
    merry_graves_encountered_error_serving(
        _MERRY_PROGRAM_ERROR_,
        _MERRY_ATTEMPT_TO_PERFORM_STATE_OPERATIONS_DURING_WREQUEST_HANDLING,
        req->base);
    return;
  }
  void *cptr = merry_graves_get_hands_on_cptr(req->base->core_id);
  if (req->base->del_state_func(cptr, req->args[1]) == RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&req->base->state);
    req->args[0] = REQUEST_FAILED;
    req->args[1] = FAILED_STATE_OPERATION;
    return;
  }
  if (req->base->active_state == req->args[1])
    req->base->active_state = (mqword_t)(-1);
  req->args[0] = REQUEST_SERVED;
}

PREQ_HDLR(HANDLE_JMP_STATE) {
  if (req->base->wrequest == mtrue) {
    merry_graves_encountered_error_serving(
        _MERRY_PROGRAM_ERROR_,
        _MERRY_ATTEMPT_TO_PERFORM_STATE_OPERATIONS_DURING_WREQUEST_HANDLING,
        req->base);
    return;
  }
  void *cptr = merry_graves_get_hands_on_cptr(req->base->core_id);
  if (req->base->jmp_state_func(cptr, req->args[1]) == RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&req->base->state);
    req->args[0] = REQUEST_FAILED;
    req->args[1] = FAILED_STATE_OPERATION;
    return;
  }
  req->args[0] = REQUEST_SERVED;
  req->base->active_state = req->args[1];
}

PREQ_HDLR(HANDLE_SWITCH_STATE) {
  if (req->base->wrequest == mtrue) {
    merry_graves_encountered_error_serving(
        _MERRY_PROGRAM_ERROR_,
        _MERRY_ATTEMPT_TO_PERFORM_STATE_OPERATIONS_DURING_WREQUEST_HANDLING,
        req->base);
    return;
  }
  void *cptr = merry_graves_get_hands_on_cptr(req->base->core_id);
  if (req->base->replace_state_func(cptr, req->args[1]) == RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&req->base->state);
    req->args[0] = REQUEST_FAILED;
    req->args[1] = FAILED_STATE_OPERATION;
    return;
  }
  req->args[0] = REQUEST_SERVED;
  req->base->active_state = req->args[1];
}

PREQ_HDLR(HANDLE_WILD_RESTORE) {
  if (req->base->wrequest == mfalse) {
    merry_graves_encountered_error_serving(
        _MERRY_PROGRAM_ERROR_,
        _MERRY_CANNOT_WILD_RESTORE_WHEN_NO_WILD_REQUEST_SERVED_, req->base);
    return;
  }
  void *cptr = merry_graves_get_hands_on_cptr(req->base->core_id);
  // restore the prior active state
  if (req->base->jmp_state_func(cptr, req->base->prior_active_state) ==
      RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&req->base->state);
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NO; // simply cannot do it. Can't you understand?
    return;
  }
  // delete that state
  req->base->del_state_func(cptr, req->base->active_state);
  // restore the prior active state
  req->base->active_state = req->base->prior_active_state;
  req->args[0] = REQUEST_SERVED;
  req->base->wrequest = req->base->stop =
      ((merry_simple_queue_empty(req->base->wild_request) ? mtrue : mfalse));
}

PREQ_HDLR(HANDLE_NEW_GROUP) {
  // make a new vcore for a new group
  MerryGravesCoreRepr *repr =
      merry_dynamic_list_at(graves.all_cores, req->base->core_id);
  if (!merry_graves_check_vcore_priviledge_or_permission(repr)) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_PRIVILEDGED;
    return;
  }
  msize_t id = 0;
  graves.group_count++;
  if (merry_graves_add_new_core(req->args[1] & __CORE_TYPE_COUNT, req->args[2],
                                -1, -1, graves.group_count, mtrue,
                                &id) == RET_FAILURE) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = FAILED_TO_ADD_CORE;
    return;
  }
  if (merry_graves_boot_core(id) == RET_FAILURE) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = FAILED_TO_ADD_CORE;
    return;
  }
  req->args[0] = REQUEST_SERVED;
  req->args[1] = id;
}

PREQ_HDLR(HANDLE_BESTOW_PRIVILEDGE) {
  if (!merry_is_vcore_id_valid(req->args[1])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = INVALID_CORE_ID;
    return;
  }
  MerryGravesCoreRepr *bestowed =
      merry_dynamic_list_at(graves.all_cores, req->args[1]);
  MerryGravesCoreRepr *bestower =
      merry_dynamic_list_at(graves.all_cores, req->base->core_id);
  req->args[0] = merry_graves_bestow_priviledge(bestower, bestowed,
                                                &req->args[1]) == RET_FAILURE
                     ? REQUEST_FAILED
                     : REQUEST_SERVED;
}

PREQ_HDLR(HANDLE_DO_NOT_DISTURB) { req->base->do_not_disturb = mtrue; }

PREQ_HDLR(HANDLE_IGNORE_PAUSE) { req->base->ignore_pause = mtrue; }

PREQ_HDLR(HANDLE_WILD_REQUEST) {
  /*
   * It is clear that just using the core id is not going to be
   * enough. As the core being requested might be dead and some other
   * core might have taken it's id so it is not a full-proof
   * identification method. Thus, for cases like these, unique
   * id will be used for making sure
   * */
  /*
   * One interesting thing one might notice is that Graves never
   * checks the status flags for each vcore. Is it in do not disturb
   * state? This is because the flags are for the vcore itself.
   * Graves doesn't have to bother itself for them.
   * */
  if (!merry_is_vcore_id_valid(req->args[1])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = INVALID_CORE_ID;
    return;
  }
  MerryGravesCoreRepr *repr =
      merry_dynamic_list_at(graves.all_cores, req->args[1]);
  if (!merry_graves_check_vcore_alive_or_dead(repr, req->args[2])) {
    // the core is already dead
    req->args[0] = REQUEST_FAILED;
    req->args[1] = CORE_DEAD;
    return;
  }
  // the core is alive
  // We need to push to the wild request queue of
  // the getter
  merry_mutex_lock(&repr->base->lock);
  if (merry_simple_queue_full(repr->base->wild_request)) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = WRQ_FULL;
    merry_mutex_unlock(&repr->base->lock);
    return;
  }
  MerryWildRequest r;
  r.request = req->args[3];
  r.requester_id = req->base->core_id;
  r.requester_uid = req->base->unique_id;
  r.arg = req->args[4];
  merry_simple_queue_enqueue(repr->base->wild_request, &r);
  repr->base->wrequest = mtrue;
  repr->base->stop = mtrue;
  merry_mutex_unlock(&repr->base->lock);
  req->args[0] = REQUEST_SERVED;
}

PREQ_HDLR(HANDLE_PAUSE) {
  MerryGravesCoreRepr *repr =
      merry_dynamic_list_at(graves.all_cores, req->args[1]);
  MerryGravesCoreRepr *this =
      merry_dynamic_list_at(graves.all_cores, req->base->core_id);
  if (!merry_graves_check_vcore_priviledge_or_permission(this)) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_PRIVILEDGED;
    return;
  }
  if (!merry_is_vcore_id_valid(req->args[1])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = INVALID_CORE_ID;
    return;
  }
  if (!merry_graves_check_vcore_alive_or_dead(repr, req->args[2])) {
    // the core is already dead
    req->args[0] = REQUEST_FAILED;
    req->args[1] = CORE_DEAD;
    return;
  }

  if (req->base->group_id != repr->base->group_id) {
    // violation of a pact
    // violator cannot continue
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_THE_SAME_GROUP;
    return;
  }
  repr->base->pause = mtrue;
  repr->base->stop = mtrue;
  req->args[0] = REQUEST_SERVED;
}

PREQ_HDLR(HANDLE_UNPAUSE) {
  MerryGravesCoreRepr *repr =
      merry_dynamic_list_at(graves.all_cores, req->args[1]);
  MerryGravesCoreRepr *this =
      merry_dynamic_list_at(graves.all_cores, req->base->core_id);
  if (!merry_graves_check_vcore_priviledge_or_permission(this)) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_PRIVILEDGED;
    return;
  }
  if (!merry_is_vcore_id_valid(req->args[1])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = INVALID_CORE_ID;
    return;
  }
  if (!merry_graves_check_vcore_alive_or_dead(repr, req->args[2])) {
    // the core is already dead
    req->args[0] = REQUEST_FAILED;
    req->args[1] = CORE_DEAD;
    return;
  }

  if (req->base->group_id != repr->base->group_id) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_THE_SAME_GROUP;
    return;
  }
  if (repr->base->pause == mtrue)
    merry_cond_signal(&repr->base->cond);
  req->args[0] = REQUEST_SERVED;
}

PREQ_HDLR(HANDLE_GRANT_PERMISSION) {
  MerryGravesCoreRepr *repr =
      merry_dynamic_list_at(graves.all_cores, req->args[1]);
  MerryGravesCoreRepr *this =
      merry_dynamic_list_at(graves.all_cores, req->base->core_id);
  if (!merry_graves_check_vcore_priviledge_or_permission(this)) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_PRIVILEDGED;
    return;
  }
  if (!merry_is_vcore_id_valid(req->args[1])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = INVALID_CORE_ID;
    return;
  }
  if (!merry_graves_check_vcore_alive_or_dead(repr, req->args[2])) {
    // the core is already dead
    req->args[0] = REQUEST_FAILED;
    req->args[1] = CORE_DEAD;
    return;
  }

  if (req->base->group_id != repr->base->group_id) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_THE_SAME_GROUP;
    return;
  }
  repr->base->permission_granted = mtrue;
  req->args[0] = REQUEST_SERVED;
}

PREQ_HDLR(HANDLE_IS_CORE_DEAD) {
  if (!merry_is_vcore_id_valid(req->args[1])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = INVALID_CORE_ID;
    return;
  }
  MerryGravesCoreRepr *core =
      merry_dynamic_list_at(graves.all_cores, req->args[1]);
  // This doesn't require permission or priviledge
  req->args[0] = REQUEST_SERVED;
  req->args[1] =
      merry_graves_check_vcore_alive_or_dead(core, req->args[2]) ? YES : NO;
}

PREQ_HDLR(HANDLE_IS_PARENT_ALIVE) {
  req->args[0] = REQUEST_SERVED;
  if (req->base->parent_core_id == (mqword_t)(-1)) {
    req->args[1] = HAS_NO_PARENT;
  } else {
    MerryGravesCoreRepr *core =
        merry_dynamic_list_at(graves.all_cores, req->base->parent_core_id);
    req->args[1] = merry_graves_check_vcore_alive_or_dead(
                       core, req->base->parent_unique_id)
                       ? YES
                       : NO;
  }
}

PREQ_HDLR(HANDLE_GET_CID) {
  req->args[0] = REQUEST_SERVED;
  req->args[1] = req->base->core_id;
}

PREQ_HDLR(HANDLE_GET_UID) {
  if (!merry_is_vcore_id_valid(req->args[1])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = INVALID_CORE_ID;
    return;
  }
  MerryGravesCoreRepr *core =
      merry_dynamic_list_at(graves.all_cores, req->args[1]);
  if (!merry_graves_check_vcore_alive_or_dead(core, req->args[2])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = CORE_DEAD;
    return;
  }

  req->args[0] = REQUEST_SERVED;
  req->args[1] = core->base->unique_id;
}

PREQ_HDLR(HANDLE_GET_GROUP) {
  if (!merry_is_vcore_id_valid(req->args[1])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = INVALID_CORE_ID;
    return;
  }
  MerryGravesCoreRepr *other =
      merry_dynamic_list_at(graves.all_cores, req->args[1]);
  req->args[0] = REQUEST_SERVED;
  req->args[1] = other->base->group_id;
}

PREQ_HDLR(HANDLE_KILL_CORE) {
  /*
   * To kill a core, the killing core must
   * 1) be priviledged(or have permission)
   * 2) have same group id as the dying core.
   * This is a controlled kill i.e intentional
   * or at least that is what i assume.
   * */
  MerryGravesCoreRepr *core =
      merry_dynamic_list_at(graves.all_cores, req->args[1]);
  MerryGravesCoreRepr *this =
      merry_dynamic_list_at(graves.all_cores, req->base->core_id);
  if (!merry_graves_check_vcore_priviledge_or_permission(this)) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_PRIVILEDGED;
    return;
  }
  if (!merry_is_vcore_id_valid(req->args[1])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = INVALID_CORE_ID;
    return;
  }
  if (!merry_graves_check_vcore_alive_or_dead(core, req->args[2])) {
    // the core is already dead
    req->args[0] = REQUEST_SERVED;
    return;
  }

  if (req->base->group_id != core->base->group_id) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_THE_SAME_GROUP;
    return;
  }
  req->args[0] = REQUEST_SERVED;
  core->base->terminate = mtrue;
  core->base->stop = mtrue;
}

PREQ_HDLR(HANDLE_IS_CHILD) {
  if (!merry_is_vcore_id_valid(req->args[1])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = INVALID_CORE_ID;
    return;
  }
  MerryGravesCoreRepr *other =
      merry_dynamic_list_at(graves.all_cores, req->args[1]);
  req->args[0] = REQUEST_SERVED;
  req->args[1] =
      (other->base->parent_unique_id == req->base->unique_id) ? YES : NO;
}

PREQ_HDLR(HANDLE_CHANGE_PARENT) {
  // A child should attempt to change it's parent
  // With permission.
  MerryGravesCoreRepr *this =
      merry_dynamic_list_at(graves.all_cores, req->base->core_id);
  if (!merry_graves_check_vcore_priviledge_or_permission(this)) {
    // This child doesn't have permission or priviledge
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_PRIVILEDGED;
    return;
  }
  // Now that the child CAN change its parent, so,
  // Check if the parent is valid
  // Check if the parent is alive
  // Check if the parent is priviledged
  // Check if belongs to the same group
  // Then perform the change
  if (!merry_is_vcore_id_valid(req->args[1])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = INVALID_CORE_ID;
    return;
  }

  MerryGravesCoreRepr *newparent =
      merry_dynamic_list_at(graves.all_cores, req->args[1]); // should not fail

  if (!merry_graves_check_vcore_alive_or_dead(newparent, req->args[2])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = CORE_DEAD;
    return;
  }

  if (!merry_graves_check_vcore_priviledge(newparent)) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NEW_PARENT_NOT_PRIVILEDGED;
    return;
  }

  if (this->base->group_id != newparent->base->group_id) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_THE_SAME_GROUP;
    return;
  }

  // finally eligible
  this->base->parent_core_id = newparent->base->core_id;
  this->base->parent_unique_id = newparent->base->unique_id;
  req->args[0] = REQUEST_SERVED;
}

PREQ_HDLR(HANDLE_CHANGE_CHILD_PARENT) {
  // some other core is trying to change the parent of
  // a child core
  // Steps:
  // - Check if the requesting(this) core is priviledged
  // - Check if the requested core valid and alive
  // - Check if the new parent is valid and alive
  // - Check that the group id of all three match
  // - Change the parent
  // Note that the requested core isn't asked for permission
  // for the change. There should be coordination among all cores.
  MerryGravesCoreRepr *this =
      merry_dynamic_list_at(graves.all_cores, req->base->core_id);
  MerryGravesCoreRepr *requested =
      merry_dynamic_list_at(graves.all_cores, req->args[1]);
  MerryGravesCoreRepr *newparent =
      merry_dynamic_list_at(graves.all_cores, req->args[3]);

  if (!merry_graves_check_vcore_priviledge_or_permission(this)) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_PRIVILEDGED;
    return;
  }
  if (!merry_is_vcore_id_valid(req->args[1]) ||
      !merry_is_vcore_id_valid(req->args[3])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = INVALID_CORE_ID;
    return;
  }

  if (!merry_graves_check_vcore_alive_or_dead(newparent, req->args[4]) ||
      !merry_graves_check_vcore_alive_or_dead(requested, req->args[2])) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = CORE_DEAD;
    return;
  }

  if (!merry_graves_check_vcore_priviledge(newparent)) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_PRIVILEDGED;
    return;
  }

  if (this->base->group_id != requested->base->group_id !=
      newparent->base->group_id) {
    req->args[0] = REQUEST_FAILED;
    req->args[1] = NOT_THE_SAME_GROUP;
    return;
  }

  // Fully eligible
  requested->base->parent_core_id = newparent->base->core_id;
  requested->base->parent_unique_id = newparent->base->unique_id;
  req->args[0] = REQUEST_SERVED;
}

PREQ_HDLR(HANDLE_GIVEUP_PRIVILEDGE) {
  // some core wants to give up its godly powers
  req->base->priviledge = mfalse; // done
  req->args[0] = REQUEST_SERVED;
}

PREQ_HDLR(HANDLE_PURGE_WREQUESTS) {
  // clear all of the wrequests
  merry_simple_queue_clear(req->base->wild_request);
  req->args[0] = REQUEST_SERVED;
}
