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

  if ((graves.core_base_func_list =
           merry_create_list(__CORE_TYPE_COUNT, sizeof(mcoredetails_t),
                             &graves.master_state)) == RET_NULL) {
    merry_MAKE_SENSE_OF_STATE(&graves.master_state);
    merry_graves_reader_destroy(graves.reader);
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

  // We need to pass the program arguments somehow.
  // We shall do it here.

  return RET_SUCCESS;
}

mret_t merry_graves_acquaint_with_cores() {
  if (merry_list_push(graves.core_base_func_list, merry_64_bit_core_base) ==
      RET_FAILURE)
    goto __error;

  return RET_SUCCESS;
__error:
  graves.master_state = graves.core_base_func_list->lstate;
  return RET_FAILURE;
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

mret_t merry_graves_add_new_core(mcore_t c_type, maddress_t begin) {
  MerryCoreBase *base;
  mcoredetails_t details;

  if ((details = merry_list_at(graves.core_base_func_list, c_type)) ==
      RET_NULL) {
    merry_assign_state(graves.master_state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FAILED_TO_ADD_CORE_);
    graves.master_state.child_state = &graves.core_base_func_list->lstate;
    return RET_FAILURE;
  }

  if ((base = details(&graves.master_state)) == RET_NULL)
    return RET_FAILURE;

  base->core_id = graves.core_count;
  void *tmp;
  if ((tmp = base->init_func(base, graves.reader->data_ram,
                             graves.reader->iram[c_type], begin)) == RET_NULL) {
    graves.master_state = base->state;
    merry_core_base_clean(base);
    return RET_FAILURE;
  }
  msize_t i;
  if (merry_graves_find_old_core(&i) == RET_FAILURE) {
    graves.core_count++;

    MerryGravesCoreRepr repr;
    repr.base = base;
    repr.cptr = tmp;

    if (merry_dynamic_list_push(graves.all_cores, &repr) == RET_FAILURE) {
      merry_assign_state(graves.master_state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_FAILED_TO_ADD_CORE_);
      graves.master_state.child_state = &graves.all_cores->lstate;
      merry_core_base_clean(base);
      return RET_FAILURE;
    }
  } else {
    MerryGravesCoreRepr *repr = merry_dynamic_list_at(graves.all_cores, i);
    repr->base = base;
    repr->cptr = tmp;
  }

  return RET_SUCCESS;
}

mret_t merry_graves_boot_core(msize_t core_id) {
  MerryGravesCoreRepr *repr;

  if ((repr = merry_dynamic_list_at(graves.all_cores, core_id)) == RET_NULL)
    return RET_FAILURE; // This will never happen

  mthread_t th; // We won't need this later unless we are creating an attached
                // thread which we are not

  if (merry_create_detached_thread(&th, repr->base->exec_func, repr->cptr,
                                   &graves.master_state) == RET_FAILURE) {
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
  repr->base->free_func(repr->cptr); // the base is cleaned up as well
  repr->cptr = NULL;
  graves.active_cores--;
  return RET_SUCCESS;
}

_THRET_T_ merry_graves_run_VM(void *arg) {

  // First start a new core
  if (merry_graves_add_new_core(graves.reader->itit.entries[0].type, 0) ==
      RET_FAILURE) {
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

  MerryGravesRequest request;

  while (graves.active_cores != 0) {
    if (merry_graves_wants_work(&request) == RET_FAILURE) {
      merry_MAKE_SENSE_OF_STATE(merry_graves_req_queue_state());
      break;
    } else {
      switch (request.type) {
      case SHUT_DOWN:
        HANDLE_SHUTDOWN(&request);
        break;
      case TRY_LOADING_NEW_PAGE_DATA:
        HANDLE_LOADING_NEW_PAGE_DATA(&request);
        break;
      case TRY_LOADING_NEW_PAGE_INST:
        HANDLE_LOADING_NEW_PAGE_INST(&request);
        break;
      case PROBLEM_ENCOUNTERED:
        HANDLE_PROBLEM_ENCOUNTERED(&request);
        break;
      case PROGRAM_REQUEST:
        HANDLE_PROGRAM_REQUEST(&request);
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
  if (!graves.core_base_func_list)
    merry_destroy_list(graves.core_base_func_list);
  if (!graves.all_cores) {
    for (msize_t i = 0; i <= graves.core_count; i++) {
      MerryGravesCoreRepr *repr = merry_dynamic_list_pop(graves.all_cores);
      if (repr->cptr != NULL)
        repr->base->free_func(repr->cptr);
    }
    merry_destroy_dynamic_list(graves.all_cores);
  }
  merry_graves_reader_destroy(graves.reader);
  merry_cond_destroy(graves.master_cond);
  merry_mutex_destroy(graves.master_lock);
  merry_graves_req_queue_free();
}

REQ_HDLR(HANDLE_SHUTDOWN) { merry_graves_clean_a_core(req->base->core_id); }

REQ_HDLR(HANDLE_LOADING_NEW_PAGE_INST) {
  if (merry_graves_reader_load_instructions(graves.reader, req->base->core_type,
                                            req->args[0] / _MERRY_PAGE_LEN_) ==
      RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&graves.reader->state);
    req->args[0] = 1;
  } else {
    req->args[0] = 0;
  }
}

REQ_HDLR(HANDLE_LOADING_NEW_PAGE_DATA) {
  if (merry_graves_reader_load_data(
          graves.reader, req->args[0] / _MERRY_PAGE_LEN_) == RET_FAILURE) {
    merry_MAKE_SENSE_OF_STATE(&graves.reader->state);
    req->args[0] = 1;
  } else {
    req->args[0] = 0;
  }
}

REQ_HDLR(HANDLE_PROBLEM_ENCOUNTERED) {
  merry_MAKE_SENSE_OF_STATE(&req->base->state);
}

REQ_HDLR(HANDLE_PROGRAM_REQUEST) {}
