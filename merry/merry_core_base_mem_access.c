#include <merry_core_base_mem_access.h>

mret_t merry_core_mem_access_GET_INST(MerryGravesRequest *req,
                                      MerryState *state, MerryRAM *ram,
                                      maddress_t addr, mqptr_t store_in) {
  if (merry_RAM_read_qword(ram, addr, store_in, state) == RET_FAILURE) {
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_INST;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_INST;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_INST;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
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
    req->args[0] = addr;
    req->type = TRY_LOADING_NEW_PAGE_DATA;
    if (merry_SEND_REQUEST(req) == RET_FAILURE || req->args[0] == 1)
      return RET_FAILURE;
    if (req->args[0] != REQUEST_SERVED)
      return RET_FAILURE;
    merry_RAM_write_qword_atm(ram, addr, to_store,
                              state); // This shouldn't fail at this point
  }
  return RET_SUCCESS;
}
