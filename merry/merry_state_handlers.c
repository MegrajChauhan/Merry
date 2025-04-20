#include "merry_state_handlers.h"

void merry_MAKE_SENSE_OF_STATE(MerryState *state) {
  merry_check_ptr(state);

  MerryState s = *state;
  merry_HANDLE_CONTEXT(s.context, s.arg);
  do {
    switch (s.origin) {
    case _MERRY_INTERNAL_SYSTEM_ERROR_:
      merry_HANDLE_INTERNAL_SYS_ERROR(s.err.sys_error, s.arg);
      break;
    case _MERRY_PROGRAM_ERROR_:
      merry_HANDLE_PROGRAM_ERROR(s.err.prog_error, s.arg);
      break;
    case _MERRY_ORIGIN_NONE_:
      break;
    }
    s = *s.child_state;
  } while (s.child_state != NULL);
}

void merry_HANDLE_INTERNAL_SYS_ERROR(msyserr_t err, MerryPtrToQword arg) {
  switch (err) {
  case _MERRY_REQUEST_TO_HOST_FAILED_:
    merry_msg("A REQUEST to the HOST OPERATING SYSTEM has failed. SYSCALL=%zu",
              arg.qword);
    break;
  case _MERRY_FAILED_TO_LOAD_DYNAMIC_LIBRARY_:
    merry_msg("Failed to LOAD a requested DYNAMIC LIBRARY[\"%s\"]",
              (mstr_t)arg.ptr);
    break;
  case _MERRY_MEM_ALLOCATION_FAILURE_:
    merry_msg("Failed to allocate MEMORY for INTERNALS.", NULL);
    break;
  case _MERRY_PAGE_FAULT_:
    merry_msg("Attempt made to access OUT OF BOUNDS memory.", NULL);
    break;
  case _MERRY_CONTAINER_RESIZE_FAILED_:
    merry_msg("Internal Container couldn't be resized.", NULL);
    break;
  case _MERRY_FAILED_TO_OBTAIN_LOCK_:
    merry_msg("Failed to OBTAIN a MUTEX LOCK.", NULL);
    break;
  case _MERRY_FAILED_TO_OBTAIN_COND_:
    merry_msg("Failed to OBTAIN a CONDITION VARIABLE.", NULL);
    break;
  case _MERRY_FAILED_TO_OPEN_PIPE_:
    merry_msg("Failed to OPEN a PIPE.", NULL);
    break;
  case _MERRY_PIPE_MISCONFIGURE_:
    merry_msg("PIPE misconfigured for the requested operation.", NULL);
    break;
  case _MERRY_FAILED_TO_SPAWN_THREAD_:
    merry_msg("Failed to SPAWN WORKER.", NULL);
    break;
  case _MERRY_GRAVES_REQUEST_QUEUE_FAILURE_:
    merry_msg("GRAVES' MEMORY Choked on requests.", NULL);
    break;
  case _MERRY_INPUT_FILE_DOESNT_EXIST_:
    merry_msg("The given input file doesn't exist.", NULL);
    break;
  case _MERRY_INPUT_FILE_IS_A_DIR_:
    merry_msg("The input file is a DIRECTORY and not a file.", NULL);
    break;
  case _MERRY_INPUT_FILE_DOESNT_FIT_FORMAT_:
    merry_msg("Unknown MAGIC NUMBER in the given INPUT FILE.", NULL);
    break;
  case _MERRY_UNKNOWN_INPUT_FILE_:
    merry_msg("The FILE is not a valid INPUT.", NULL);
    break;
  case _MERRY_CANNOT_DEDUCE_FILE_TYPE_:
    merry_msg("Unsupported or Unknown FILE TYPE.", NULL);
    break;
  case _MERRY_MISALIGNED_ITIT_HEADER_:
    merry_msg(
        "ITIT Header suggests misaligned ITIT which must be 16-byte aligned.",
        NULL);
    break;
  case _MERRY_MISALIGNED_SIT_HEADER_:
    merry_msg(
        "SIT Header suggests misaligned SIT which must be 16-byte aligned.",
        NULL);
    break;
  case _MERRY_INVALID_FILE_STRUCTURE_DOESNT_MATCH_HEADER_INFO_:
    merry_msg("The HEADER information doesn't complement the obtained file "
              "information.",
              NULL);
    break;
  case _MERRY_FRAGMENTED_INSTRUCTION_SECTION_:
    merry_msg("Instructions for same CORE TYPE must be in the same INSTRUCTION "
              "SECTION. Fragmentation Seen.",
              NULL);
    break;
  case _MERRY_MISALIGNED_INSTRUCTION_SECTION_:
    merry_msg("Instructions must be 8-byte aligned. Misaligned instruction "
              "sections found.",
              NULL);
    break;
  case _MERRY_INSTRUCTION_SECTION_CANNOT_BE_ZERO_:
    merry_msg("Instruction sections cannot have a length of 0 bytes.", NULL);
    break;
  }
}

void merry_HANDLE_PROGRAM_ERROR(mprogerr_t err, MerryPtrToQword arg) {
  switch (err) {
  case _DIV_BY_ZERO_:
    merry_msg("CORE=%zu: Attempt to divide by zero.", arg.qword);
    break;
  }
}

void merry_HANDLE_CONTEXT(mcont_t cont, MerryPtrToQword arg) {
  switch (cont) {
  case _MERRY_CORE_BASE_INITIALIZATION_: {
    merry_msg("While INITIALIZING a new CORE BASE:", NULL);
    break;
  }
  case _MERRY_CORE_INITIALIZATION_: {
    merry_msg("While INITIALIZING CORE[ID: %zu]:", arg.qword);
    break;
  }
  }
}
