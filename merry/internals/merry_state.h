#ifndef _MERRY_STATE_
#define _MERRY_STATE_

#define merry_assign_state(state, orig, type) \
    do                                        \
    {                                         \
        (state).origin = (orig);              \
        (state).err.__ = (type);              \
    } while (0)

typedef enum merrOrigin_t merrOrigin_t;
typedef enum msyserr_t msyserr_t;
typedef enum mprogerr_t mprogerr_t;

typedef struct MerryState MerryState; // This signifies the state of an active component

enum merrOrigin_t
{
    _MERRY_ORIGIN_NONE_,           // symbolizing a state of no error
    _MERRY_INTERNAL_SYSTEM_ERROR_, // The internal system caused the problem
    _MERRY_PROGRAM_ERROR_,         // the running program is the problem
};

enum msyserr_t
{
    _MERRY_REQUEST_TO_HOST_FAILED_, // Some request made to the OS failed
    _MERRY_MEM_ALLOCATION_FAILURE_, // stdlid failed
    _MERRY_PAGE_FAULT_,             // accessing page memory that doesn't exist
};

enum mprogerr_t
{
    _DIV_BY_ZERO_,
};

struct MerryState
{
    merrOrigin_t origin;
    union
    {
        msyserr_t sys_error;
        mprogerr_t prog_error;
        msize_t __; // value representation
    } err;
};

#endif