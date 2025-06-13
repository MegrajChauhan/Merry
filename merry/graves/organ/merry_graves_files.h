#ifndef _MERRY_GRAVES_FILES_
#define _MERRY_GRAVES_FILES_

#include <merry_core_base_mem_access.h>
#include <merry_files.h>
#include <merry_graves_request_queue.h>
#include <merry_state.h>
#include <merry_types.h>
#include <merry_utils.h>

// All mexecret_t functions
mret_t merry_graves_open_file(MerryCoreBase *base, MerryGravesExecReturn *dets,
                              MerryRAM *iram, MerryRAM *dram);

mret_t merry_graves_close_file(MerryCoreBase *base, MerryGravesExecReturn *dets,
                               MerryRAM *iram, MerryRAM *dram);

mret_t merry_graves_read_file(MerryCoreBase *base, MerryGravesExecReturn *dets,
                              MerryRAM *iram, MerryRAM *dram);

mret_t merry_graves_write_file(MerryCoreBase *base, MerryGravesExecReturn *dets,
                               MerryRAM *iram, MerryRAM *dram);

#endif
