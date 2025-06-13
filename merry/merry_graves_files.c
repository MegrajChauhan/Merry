#include "merry_graves_files.h"

/**
 * Since the vcores themselves execute these functions, we must
 * assume them as being part of the vcore module.
 * */

mret_t merry_graves_open_file(MerryCoreBase *base, MerryGravesExecReturn *dets,
                              MerryRAM *iram, MerryRAM *dram) {
  merry_check_ptr(base);
  merry_check_ptr(dets);
  MerryFile *file = (MerryFile *)dets->arg;

  if (file->open == mtrue) {
    merry_assign_state(file->state, _MERRY_PROGRAM_ERROR_,
                       _MERRY_TRYING_TO_OVERWRITE_ALREADY_OPEN_FILE_);
    return RET_FAILURE;
  }

  // Getting the path
  mbptr_t path;
  MerryGravesRequest req;
  req.base = base;
  if (merry_core_base_mem_access_bulk_read(&req, &file->state, dram,
                                           dets->args[0], &path,
                                           dets->args[1]) == RET_FAILURE) {
  }

  if (merry_open_file(file, , mstr_t modes))

    return RET_SUCCESS;
}

mret_t merry_graves_close_file(MerryCoreBase *base, MerryGravesExecReturn *dets,
                               MerryRAM *iram, MerryRAM *dram);

mret_t merry_graves_read_file(MerryCoreBase *base, MerryGravesExecReturn *dets,
                              MerryRAM *iram, MerryRAM *dram);

mret_t merry_graves_write_file(MerryCoreBase *base, MerryGravesExecReturn *dets,
                               MerryRAM *iram, MerryRAM *dram);
