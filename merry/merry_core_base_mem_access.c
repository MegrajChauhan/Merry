#include <merry_core_base_mem_access.h>

_MERRY_INTERNAL_ mret_t merry_core_mem_access_understand_state(
    MerryState *state, MerryGravesRequest *req, mgreq_t r, maddress_t addr) {
  if (state->err.sys_error == _MERRY_PAGE_FAULT_) {
    req->type = r;
    req->args[0] = addr;
    if (merry_SEND_REQUEST(req) == RET_FAILURE)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
      return RET_FAILURE;
  } else {
    req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(req);
    return RET_FAILURE;
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_GET_INST(MerryGravesRequest *req,
                                      MerryState *state, MerryRAM *ram,
                                      maddress_t addr, mqptr_t store_in) {
  if (merry_RAM_read_qword(ram, addr, store_in, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_INST, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_read_qword(ram, addr, store_in,
                         state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_GET_DATA_byte(MerryGravesRequest *req,
                                           MerryState *state, MerryRAM *ram,
                                           maddress_t addr, mbptr_t store_in) {
  if (merry_RAM_read_byte(ram, addr, store_in, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_read_byte(ram, addr, store_in,
                        state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_GET_DATA_word(MerryGravesRequest *req,
                                           MerryState *state, MerryRAM *ram,
                                           maddress_t addr, mwptr_t store_in) {
  if (merry_RAM_read_word(ram, addr, store_in, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_read_word(ram, addr, store_in,
                        state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_GET_DATA_dword(MerryGravesRequest *req,
                                            MerryState *state, MerryRAM *ram,
                                            maddress_t addr, mdptr_t store_in) {
  if (merry_RAM_read_dword(ram, addr, store_in, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_read_dword(ram, addr, store_in,
                         state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_GET_DATA_qword(MerryGravesRequest *req,
                                            MerryState *state, MerryRAM *ram,
                                            maddress_t addr, mqptr_t store_in) {
  if (merry_RAM_read_qword(ram, addr, store_in, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_read_qword(ram, addr, store_in,
                         state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_GET_DATA_byte_atm(MerryGravesRequest *req,
                                               MerryState *state, MerryRAM *ram,
                                               maddress_t addr,
                                               mbptr_t store_in) {
  if (merry_RAM_read_byte_atm(ram, addr, store_in, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_read_byte_atm(ram, addr, store_in,
                            state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_GET_DATA_word_atm(MerryGravesRequest *req,
                                               MerryState *state, MerryRAM *ram,
                                               maddress_t addr,
                                               mwptr_t store_in) {
  if (merry_RAM_read_word_atm(ram, addr, store_in, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_read_word_atm(ram, addr, store_in,
                            state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_GET_DATA_dword_atm(MerryGravesRequest *req,
                                                MerryState *state,
                                                MerryRAM *ram, maddress_t addr,
                                                mdptr_t store_in) {
  if (merry_RAM_read_dword_atm(ram, addr, store_in, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_read_dword_atm(ram, addr, store_in,
                             state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_GET_DATA_qword_atm(MerryGravesRequest *req,
                                                MerryState *state,
                                                MerryRAM *ram, maddress_t addr,
                                                mqptr_t store_in) {
  if (merry_RAM_read_qword_atm(ram, addr, store_in, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_read_qword_atm(ram, addr, store_in,
                             state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_WRITE_DATA_byte(MerryGravesRequest *req,
                                             MerryState *state, MerryRAM *ram,
                                             maddress_t addr,
                                             mbyte_t to_store) {
  if (merry_RAM_write_byte(ram, addr, to_store, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_write_byte(ram, addr, to_store,
                         state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_WRITE_DATA_word(MerryGravesRequest *req,
                                             MerryState *state, MerryRAM *ram,
                                             maddress_t addr,
                                             mword_t to_store) {
  if (merry_RAM_write_word(ram, addr, to_store, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_write_word(ram, addr, to_store,
                         state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_WRITE_DATA_dword(MerryGravesRequest *req,
                                              MerryState *state, MerryRAM *ram,
                                              maddress_t addr,
                                              mdword_t to_store) {
  if (merry_RAM_write_dword(ram, addr, to_store, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_write_dword(ram, addr, to_store,
                          state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_WRITE_DATA_qword(MerryGravesRequest *req,
                                              MerryState *state, MerryRAM *ram,
                                              maddress_t addr,
                                              mqword_t to_store) {
  if (merry_RAM_write_qword(ram, addr, to_store, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_write_qword(ram, addr, to_store,
                          state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_WRITE_DATA_byte_atm(MerryGravesRequest *req,
                                                 MerryState *state,
                                                 MerryRAM *ram, maddress_t addr,
                                                 mbyte_t to_store) {
  if (merry_RAM_write_byte_atm(ram, addr, to_store, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_write_byte_atm(ram, addr, to_store,
                             state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_WRITE_DATA_word_atm(MerryGravesRequest *req,
                                                 MerryState *state,
                                                 MerryRAM *ram, maddress_t addr,
                                                 mword_t to_store) {
  if (merry_RAM_write_word_atm(ram, addr, to_store, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_write_word_atm(ram, addr, to_store,
                             state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_WRITE_DATA_dword_atm(MerryGravesRequest *req,
                                                  MerryState *state,
                                                  MerryRAM *ram,
                                                  maddress_t addr,
                                                  mdword_t to_store) {
  if (merry_RAM_write_dword_atm(ram, addr, to_store, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;
    merry_RAM_write_dword_atm(ram, addr, to_store,
                              state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_mem_access_WRITE_DATA_qword_atm(MerryGravesRequest *req,
                                                  MerryState *state,
                                                  MerryRAM *ram,
                                                  maddress_t addr,
                                                  mqword_t to_store) {
  if (merry_RAM_write_qword_atm(ram, addr, to_store, state) == RET_FAILURE) {
    if (merry_core_mem_access_understand_state(
            state, req, TRY_LOADING_NEW_PAGE_DATA, addr) == RET_FAILURE)
      return RET_FAILURE;

    merry_RAM_write_qword_atm(ram, addr, to_store,
                              state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}

mret_t merry_core_base_mem_access_bulk_read(MerryGravesRequest *req,
                                            MerryState *state, MerryRAM *ram,
                                            maddress_t addr, mbptr_t *buf,
                                            msize_t n) {
  register mqword_t tolerance =
      n / _MERRY_PAGE_LEN_ + ((n % _MERRY_PAGE_LEN_) > 0 ? 1 : 0);

  if ((*buf = merry_RAM_bulk_read(ram, addr, n, state)) == RET_NULL) {
    if (state->err.sys_error == _MERRY_PAGE_FAULT_) {
      for (msize_t i = 0; i < tolerance; i++) {
        req->type = TRY_LOADING_NEW_PAGE_DATA;
        req->args[0] = addr + i * _MERRY_PAGE_LEN_;
        if (merry_SEND_REQUEST(req) == RET_FAILURE)
          return RET_FAILURE;
      }
    } else {
      req->type = PROBLEM_ENCOUNTERED;
      merry_SEND_REQUEST(req);
      return RET_FAILURE;
    }
    *buf = merry_RAM_bulk_read(ram, addr, n,
                               state); // this shouldn't fail now
  }
  return RET_SUCCESS;
}

mret_t merry_core_base_mem_access_bulk_write(MerryGravesRequest *req,
                                             MerryState *state, MerryRAM *ram,
                                             maddress_t addr, mbptr_t buf,
                                             msize_t n) {
  register mqword_t tolerance =
      n / _MERRY_PAGE_LEN_ + ((n % _MERRY_PAGE_LEN_) > 0 ? 1 : 0);

  if (merry_RAM_bulk_write(ram, addr, n, buf, state) == RET_FAILURE) {

    if (state->err.sys_error == _MERRY_PAGE_FAULT_) {
      for (msize_t i = 0; i < tolerance; i++) {
        req->type = TRY_LOADING_NEW_PAGE_DATA;
        req->args[0] = addr + i * _MERRY_PAGE_LEN_;
        if (merry_SEND_REQUEST(req) == RET_FAILURE) {
          return RET_FAILURE;
        }
      }
    } else {
      req->type = PROBLEM_ENCOUNTERED;
      merry_SEND_REQUEST(req);
      return RET_FAILURE;
    }
    merry_RAM_bulk_write(ram, addr, n, buf,
                         state); // this shouldn't fail now
  }
  return RET_SUCCESS;
}
