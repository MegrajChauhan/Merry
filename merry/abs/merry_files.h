#ifndef _MERRY_FILES_
#define _MERRY_FILES_

#include <merry_platform.h>
#include <merry_state.h>
#include <merry_types.h>
#include <merry_utils.h>
#include <stdlib.h>

typedef struct MerryFile MerryFile;

struct MerryFile {
  FILE *fd;
  MerryState state;
  mbool_t trigger_on_read;
  mbool_t trigger_on_write;
  mbool_t trigger_on_open;
  mbool_t trigger_on_close;
  mbool_t open;
  msize_t last_rw_size;
};

MerryFile *merry_pre_open_file(MerryState *state, mbool_t tor, mbool_t tow,
                               mbool_t too, mbool_t toc);

mret_t merry_open_file(MerryFile *file, mstr_t file_path, mstr_t modes);

mret_t merry_close_file(MerryFile *file);

mret_t merry_file_read(MerryFile *file, mptr_t buffer, msize_t n, msize_t size);

mret_t merry_file_write(MerryFile *file, mptr_t buffer, msize_t n,
                        msize_t size);

#endif
