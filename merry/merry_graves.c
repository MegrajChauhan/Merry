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
                                 mbool_t priviledge, msize_t *id) {
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
  return RET_SUCCESS;
}

mret_t merry_graves_bestow_priviledge(msize_t bestower, msize_t bestowed) {
  MerryGravesCoreRepr *_bestower =
      merry_dynamic_list_at(graves.all_cores, bestower);
  // if bestowed is invalid, we fail silently
  if (!(merry_dyn_list_has_at_least(graves.all_cores, bestowed + 1)))
    return RET_FAILURE;
  MerryGravesCoreRepr *_bestowed =
      merry_dynamic_list_at(graves.all_cores, bestowed);
  if (_bestower->base->priviledge != mtrue) {
    merry_assign_state(_bestower->base->state, _MERRY_PROGRAM_ERROR_,
                       _MERRY_NOT_PRIVILEDGED_FOR_THIS_OPERATION_);
    merry_provide_context(_bestower->base->state,
                          _MERRY_GRAVES_SERVING_REQUEST_);
    _bestower->base->state.arg.qword = _bestower->base->core_id;
    merry_MAKE_SENSE_OF_STATE(&_bestower->base->state);
    // bestower cannot continue working anymore.
    _bestower->base->stop = mtrue;
    _bestower->base->do_not_disturb = mtrue;
    return RET_FAILURE;
  }
  atomic_store((atomic_bool *)&_bestowed->base->priviledge, mtrue);
  return RET_SUCCESS;
}

mbool_t merry_graves_check_vcore_priviledge_or_permission(msize_t id) {
  MerryGravesCoreRepr *core = merry_dynamic_list_at(graves.all_cores, id);
  return (core != NULL && ((core->base->priviledge == mtrue) ||
                           (core->base->permission_granted == mtrue)));
}

void merry_graves_encountered_error_serving(merrOrigin_t orig, mqword_t err,
                                            MerryCoreBase *base) {
  merry_assign_state(base->state, orig, err);
  merry_provide_context(base->state, _MERRY_GRAVES_SERVING_REQUEST_);
  base->state.arg.qword = base->core_id;
  merry_MAKE_SENSE_OF_STATE(&base->state);
  base->do_not_disturb = mtrue;
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
                                mtrue, NULL) == RET_FAILURE) {
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
  }
}

/*------Handling Program Requests------*/

PREQ_HDLR(HANDLE_NEW_THREAD) {
  if (!merry_graves_check_vcore_priviledge_or_permission(req->base->core_id)) {
    merry_graves_encountered_error_serving(
        _MERRY_PROGRAM_ERROR_, _MERRY_NOT_PRIVILEDGED_FOR_THIS_OPERATION_,
        req->base);
    return;
  }
  msize_t id = 0;
  if (merry_graves_add_new_core(req->args[1] & __CORE_TYPE_COUNT, req->args[2],
                                req->base->core_id, req->base->unique_id,
                                req->args[3] & 0xFF, &id) == RET_FAILURE) {
    req->args[0] = FAILED_TO_ADD_CORE;
    return;
  }
  if (merry_graves_boot_core(id) == RET_FAILURE) {
    req->args[0] = FAILED_TO_ADD_CORE;
    return;
  }
  req->args[0] = REQUEST_SERVED;
  req->args[1] = id;
}

PREQ_HDLR(HANDLE_ADD_A_NEW_DATA_MEMORY_PAGE) {
  if (!merry_graves_check_vcore_priviledge_or_permission(req->base->core_id)) {
    merry_graves_encountered_error_serving(
        _MERRY_PROGRAM_ERROR_, _MERRY_NOT_PRIVILEDGED_FOR_THIS_OPERATION_,
        req->base);
    return;
  }
  msize_t tmp = req->base->ram->page_count;

  if (merry_RAM_add_pages(req->base->ram, req->args[1], &graves.master_state) ==
      RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&graves.master_state);
    req->args[0] = FAILED_TO_ADD_DATA_MEMORY_PAGE;
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
  }
  req->args[0] = REQUEST_SERVED;
  req->base->active_state = req->args[1];
}

PREQ_HDLR(HANDLE_WILD_RESTORE) {
  if (req->base->wrequest == mfalse) {
    merry_graves_encountered_error_serving(
        _MERRY_PROGRAM_ERROR_,
        _MERRY_CANNOT_WILD_RESTORE_WHEN_NO_WILD_REQUEST_SERVED_, req->base);
    // we do not clear the "occupied" mark
    // the vcore must terminate
    // even clearing "occupied" won't have any consequences
    // since "do_not_disturb" is set
    return;
  }
  void *cptr = merry_graves_get_hands_on_cptr(req->base->core_id);
  // restore the prior active state
  if (req->base->jmp_state_func(cptr, req->base->prior_active_state) ==
      RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&req->base->state);
    req->args[0] = REQUEST_FAILED;
  }
  // delete that state
  req->base->del_state_func(cptr, req->base->active_state);
  // restore the prior active state
  req->base->active_state = req->base->prior_active_state;
  req->args[0] = REQUEST_SERVED;
  req->base->wrequest = mfalse;
}
