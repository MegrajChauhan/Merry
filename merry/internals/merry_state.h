#ifndef _MERRY_STATE_
#define _MERRY_STATE_

#include "merry_helpers.h"
#include <merry_types.h>

#define merry_assign_state(state, orig, type)                                  \
  do {                                                                         \
    (state).origin = (orig);                                                   \
    (state).err.__ = (type);                                                   \
  } while (0)

#define merry_provide_context(state, cont)                                     \
  do {                                                                         \
    (state).context = (cont);                                                  \
  } while (0)

typedef enum merrOrigin_t merrOrigin_t;
typedef enum msyserr_t msyserr_t;
typedef enum mprogerr_t mprogerr_t;
typedef enum mcont_t
    mcont_t; // couldn't use mcontext_t since the kernel apparently uses it

typedef struct MerryState
    MerryState; // This signifies the state of an active component

enum mcont_t {
  _MERRY_CONT_NONE_,
  _MERRY_CORE_BASE_INITIALIZATION_,
  _MERRY_CORE_INITIALIZATION_,
  _MERRY_GRAVES_INITIALIZATION_,
  _MERRY_GRAVES_SERVING_REQUEST_,
  _MERRY_GRAVES_BOOTING_A_CORE_, // Could be serving a request
  _MERRY_CORE_EXECUTING_,
};

enum merrOrigin_t {
  _MERRY_ORIGIN_NONE_,           // symbolizing a state of no error
  _MERRY_INTERNAL_SYSTEM_ERROR_, // The internal system caused the problem
  _MERRY_PROGRAM_ERROR_,         // the running program is the problem
};

enum msyserr_t {
  _MERRY_REQUEST_TO_HOST_FAILED_, // Some request made to the OS failed
  _MERRY_FAILED_TO_LOAD_DYNAMIC_LIBRARY_,
  _MERRY_MEM_ALLOCATION_FAILURE_, // stdlid failed
  _MERRY_PAGE_FAULT_,             // accessing page memory that doesn't exist
  _MERRY_CONTAINER_RESIZE_FAILED_,
  _MERRY_FAILED_TO_OBTAIN_LOCK_,
  _MERRY_FAILED_TO_OBTAIN_COND_,
  _MERRY_FAILED_TO_OPEN_PIPE_,
  _MERRY_PIPE_MISCONFIGURE_,
  _MERRY_FAILED_TO_SPAWN_THREAD_,
  _MERRY_GRAVES_REQUEST_QUEUE_FAILURE_,
  _MERRY_INPUT_FILE_DOESNT_EXIST_,
  _MERRY_INPUT_FILE_IS_A_DIR_,
  _MERRY_INPUT_FILE_DOESNT_FIT_FORMAT_,
  _MERRY_UNKNOWN_INPUT_FILE_,      // magic numbers don't match
  _MERRY_CANNOT_DEDUCE_FILE_TYPE_, // File Type is unknown
  _MERRY_MISALIGNED_ITIT_HEADER_,  // The ITIT header is not valid
  _MERRY_MISALIGNED_SIT_HEADER_,
  _MERRY_MISALIGNED_INSTRUCTION_SECTION_,
  _MERRY_MISALIGNED_DATA_SECTION_,
  _MERRY_FRAGMENTED_INSTRUCTION_SECTION_,
  _MERRY_INSTRUCTION_SECTION_CANNOT_BE_ZERO_,
  _MERRY_BULK_OPERATION_CANNOT_BE_DONE_ON_LENGTH_ZERO_,
  _MERRY_INVALID_FILE_STRUCTURE_DOESNT_MATCH_HEADER_INFO_,
  _MERRY_FAILED_TO_ADD_CORE_,
  _MERRY_STACK_OVERFLOW_,
  _MERRY_STACK_UNDERFLOW_,
  _MERRY_FAILED_TO_OPEN_FILE_,
  _MERRY_CANNOT_CREATE_NEW_STATE_,
};

enum mprogerr_t {
  _DIV_BY_ZERO_,
  _INVALID_PROCEDURE_RETURN_,
  _INVALID_STACK_ACCESS_,
  _MERRY_TRYING_TO_OVERWRITE_ALREADY_OPEN_FILE_, // The file descriptor is
                                                 // already being used but
                                                 // trying to re-open a file.
  _MERRY_INVALID_STATE_ID_,
  _MERRY_CANNOT_WILD_RESTORE_WHEN_NO_WILD_REQUEST_SERVED_,
  _MERRY_ATTEMPT_TO_PERFORM_STATE_OPERATIONS_DURING_WREQUEST_HANDLING,
  _MERRY_NOT_PRIVILEDGED_FOR_THIS_OPERATION_,
  _MERRY_GROUPS_DONT_MATCH_,
};

struct MerryState {
  merrOrigin_t origin;
  mcont_t context;
  union {
    msyserr_t sys_error;
    mprogerr_t prog_error;
    msize_t __; // value representation
  } err;
  MerryState *child_state;
  MerryPtrToQword arg;
};

#endif
