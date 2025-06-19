#include "merry_files.h"

MerryFile *merry_pre_open_file(MerryState *state, mbool_t tor, mbool_t tow,
                               mbool_t too, mbool_t toc) {
  MerryFile *file = (MerryFile *)malloc(sizeof(MerryFile));
  if (!file) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    return RET_NULL;
  }

  file->fd = NULL; // Is invalid to use.
  file->open = mfalse;
  file->trigger_on_open = too;
  file->trigger_on_close = toc;
  file->trigger_on_read = tor;
  file->trigger_on_write = tow;
  merry_assign_state(file->state, _MERRY_ORIGIN_NONE_, 0);
  return file;
}

mret_t merry_open_file(MerryFile *file, mstr_t file_path, mstr_t modes) {
  merry_check_ptr(file);
  merry_check_ptr(file_path);
  merry_check_ptr(modes);

  // unless by error in code logic, an already open file is never
  // reopened
  if ((file->fd = fopen(file_path, modes)) == RET_NULL) {
    merry_assign_state(file->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FAILED_TO_OPEN_FILE_);
    return RET_FAILURE;
  }

  file->open = mtrue;
  return RET_SUCCESS;
}

mret_t merry_close_file(MerryFile *file) {
  merry_check_ptr(file);
  merry_check_ptr(file->fd);

  fclose(file->fd);
  free(file);
  return RET_SUCCESS;
}

mret_t merry_file_read(MerryFile *file, mptr_t buffer, msize_t n,
                       msize_t size) {
  merry_check_ptr(file);
  merry_check_ptr(file->fd);
  merry_check_ptr(buffer);
  merry_assert(n != 0);
  merry_assert(size != 0);

  file->last_rw_size = fread(buffer, size, n, file->fd);

  return RET_SUCCESS;
}

mret_t merry_file_write(MerryFile *file, mptr_t buffer, msize_t n,
                        msize_t size) {
  merry_check_ptr(file);
  merry_check_ptr(file->fd);
  merry_check_ptr(buffer);
  merry_assert(n != 0);
  merry_assert(size != 0);

  file->last_rw_size = fwrite(buffer, size, n, file->fd);

  return RET_SUCCESS;
}
