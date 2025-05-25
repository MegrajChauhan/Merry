#include "merry_state_handlers.h"

void merry_MAKE_SENSE_OF_STATE(MerryState *state) {
  merry_check_ptr(state);

  MerryState *s = state;
  merry_HANDLE_CONTEXT(s->context, s->arg);
  do {
    switch (s->origin) {
    case _MERRY_INTERNAL_SYSTEM_ERROR_:
      merry_HANDLE_INTERNAL_SYS_ERROR(s->err.sys_error, s->arg);
      break;
    case _MERRY_PROGRAM_ERROR_:
      merry_HANDLE_PROGRAM_ERROR(s->err.prog_error, s->arg);
      break;
    case _MERRY_ORIGIN_NONE_:
      break;
    }
    merry_assign_state(*s, _MERRY_ORIGIN_NONE_, 0);
    merry_provide_context(*s, _MERRY_CONT_NONE_);
    s = s->child_state;
  } while (s != NULL);
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
  case _MERRY_MISALIGNED_DATA_SECTION_:
    merry_msg("Data section must be 8-byte, 4-byte or 2-byte aligned. "
              "Misaligned data "
              "sections found.",
              NULL);
    break;
  case _MERRY_INSTRUCTION_SECTION_CANNOT_BE_ZERO_:
    merry_msg("Instruction sections cannot have a length of 0 bytes.", NULL);
    break;
  case _MERRY_BULK_OPERATION_CANNOT_BE_DONE_ON_LENGTH_ZERO_:
    merry_msg("Cannot provide a BULK READ/WRITE with the load size of 0.",
              NULL);
    break;
  case _MERRY_FAILED_TO_ADD_CORE_:
    merry_msg("Failed to ADD a new EXECUTING CORE.", NULL);
    break;
  case _MERRY_STACK_OVERFLOW_:
    merry_msg("STACK Overflowed: WISE USE of the STACK is advised.", NULL);
    break;
  case _MERRY_STACK_UNDERFLOW_:
    merry_msg("STACK Underflowed: POPPING from STACK when there is NOTHING is "
              "not good!",
              NULL);
    break;
  }
}

void merry_HANDLE_PROGRAM_ERROR(mprogerr_t err, MerryPtrToQword arg) {
  switch (err) {
  case _DIV_BY_ZERO_:
    merry_msg("CORE=%zu: Attempt to divide by zero.", arg.qword);
    break;
  case _INVALID_PROCEDURE_RETURN_:
    merry_msg("INVALID RETURN: The CORE didn't know where to RETURN to when it "
              "didn't jump before.",
              NULL);
    break;
  case _INVALID_STACK_ACCESS_:
    merry_msg("STACK IS FED UP: The address you used to access the STACK is "
              "out of bounds.",
              NULL);
    break;
  }
}

void merry_HANDLE_CONTEXT(mcont_t cont, MerryPtrToQword arg) {
  switch (cont) {
  case _MERRY_CONT_NONE_:
    break;
  case _MERRY_CORE_BASE_INITIALIZATION_: {
    merry_msg("While INITIALIZING a new CORE BASE:", NULL);
    break;
  }
  case _MERRY_CORE_INITIALIZATION_: {
    merry_msg("While INITIALIZING CORE[ID: %zu]:", arg.qword);
    break;
  }
  case _MERRY_GRAVES_INITIALIZATION_:
    merry_msg("While INITIALIZING the MANAGER GRAVES..", NULL);
    break;
  case _MERRY_GRAVES_SERVING_REQUEST_:
    merry_msg("While SERVING a request[REQ_ID=%zu]:", arg.qword);
    break;
  case _MERRY_GRAVES_BOOTING_A_CORE_:
    merry_msg("While BOOTING UP a NEW CORE[CORE_ID=%zu]:", arg.qword);
    break;
  case _MERRY_CORE_EXECUTING_:
    merry_msg("While a CORE was EXECUTING[CORE_ID=%zu]:", arg.qword);
    break;
  }
}
