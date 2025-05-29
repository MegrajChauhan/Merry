#ifndef _MERRY_CORE_BASE_MEM_ACCESS_
#define _MERRY_CORE_BASE_MEM_ACCESS_

#include <merry_core_base.h>
#include <merry_graves_request_queue.h>
#include <merry_ram.h>

mret_t merry_core_mem_access_GET_INST(MerryGravesRequest *req,
                                      MerryState *state, MerryRAM *ram,
                                      maddress_t addr, mqptr_t store_in);

mret_t merry_core_mem_access_GET_DATA_byte(MerryGravesRequest *req,
                                           MerryState *state, MerryRAM *ram,
                                           maddress_t addr, mbptr_t store_in);

mret_t merry_core_mem_access_GET_DATA_word(MerryGravesRequest *req,
                                           MerryState *state, MerryRAM *ram,
                                           maddress_t addr, mwptr_t store_in);

mret_t merry_core_mem_access_GET_DATA_dword(MerryGravesRequest *req,
                                            MerryState *state, MerryRAM *ram,
                                            maddress_t addr, mdptr_t store_in);

mret_t merry_core_mem_access_GET_DATA_qword(MerryGravesRequest *req,
                                            MerryState *state, MerryRAM *ram,
                                            maddress_t addr, mqptr_t store_in);

mret_t merry_core_mem_access_GET_DATA_byte_atm(MerryGravesRequest *req,
                                               MerryState *state, MerryRAM *ram,
                                               maddress_t addr,
                                               mbptr_t store_in);

mret_t merry_core_mem_access_GET_DATA_word_atm(MerryGravesRequest *req,
                                               MerryState *state, MerryRAM *ram,
                                               maddress_t addr,
                                               mwptr_t store_in);

mret_t merry_core_mem_access_GET_DATA_dword_atm(MerryGravesRequest *req,
                                                MerryState *state,
                                                MerryRAM *ram, maddress_t addr,
                                                mdptr_t store_in);

mret_t merry_core_mem_access_GET_DATA_qword_atm(MerryGravesRequest *req,
                                                MerryState *state,
                                                MerryRAM *ram, maddress_t addr,
                                                mqptr_t store_in);

mret_t merry_core_mem_access_WRITE_DATA_byte(MerryGravesRequest *req,
                                             MerryState *state, MerryRAM *ram,
                                             maddress_t addr, mbyte_t to_store);

mret_t merry_core_mem_access_WRITE_DATA_word(MerryGravesRequest *req,
                                             MerryState *state, MerryRAM *ram,
                                             maddress_t addr, mword_t to_store);

mret_t merry_core_mem_access_WRITE_DATA_dword(MerryGravesRequest *req,
                                              MerryState *state, MerryRAM *ram,
                                              maddress_t addr,
                                              mdword_t to_store);

mret_t merry_core_mem_access_WRITE_DATA_qword(MerryGravesRequest *req,
                                              MerryState *state, MerryRAM *ram,
                                              maddress_t addr,
                                              mqword_t to_store);

mret_t merry_core_mem_access_WRITE_DATA_byte_atm(MerryGravesRequest *req,
                                                 MerryState *state,
                                                 MerryRAM *ram, maddress_t addr,
                                                 mbyte_t to_store);

mret_t merry_core_mem_access_WRITE_DATA_word_atm(MerryGravesRequest *req,
                                                 MerryState *state,
                                                 MerryRAM *ram, maddress_t addr,
                                                 mword_t to_store);

mret_t merry_core_mem_access_WRITE_DATA_dword_atm(MerryGravesRequest *req,
                                                  MerryState *state,
                                                  MerryRAM *ram,
                                                  maddress_t addr,
                                                  mdword_t to_store);

mret_t merry_core_mem_access_WRITE_DATA_qword_atm(MerryGravesRequest *req,
                                                  MerryState *state,
                                                  MerryRAM *ram,
                                                  maddress_t addr,
                                                  mqword_t to_store);

#endif
