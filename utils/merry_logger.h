#ifndef _MERRY_LOGGER_
#define _MERRY_LOGGER_

#include <stdio.h>
#include "../sys/merry_thread.h"

// #define _MERRY_LOGGER_ENABLED_

#if defined(_MERRY_LOGGER_ENABLED_)
static FILE *f = NULL;
static MerryMutex *lock = NULL;

// we expect that the fopen and mutex init doesn't fail
#define merry_logger_init()             \
    do                                  \
    {                                   \
        f = fopen("merryLog.txt", "w"); \
        lock = merry_mutex_init();      \
    } while (0);

#define merry_logger_close()       \
    do                             \
    {                              \
        fclose(f);                 \
        merry_mutex_destroy(lock); \
    } while (0);

// The logging format is: [Device][Additional Information]: Details
#define merry_log(_device_, _info_, _details_)                      \
    do                                                              \
    {                                                               \
        merry_mutex_lock(lock);                                     \
        fprintf(f, "[%s][%s]: %s.\n", _device_, _info_, _details_); \
        merry_mutex_unlock(lock);                                   \
    } while (0);

#define merry_llog(_device_, _info_, _msg_, ...)   \
    do                                             \
    {                                              \
        merry_mutex_lock(lock);                    \
        fprintf(f, "[%s][%s]:", _device_, _info_); \
        fprintf(f, _msg_, __VA_ARGS__);            \
        putc((int)'\n', f);                        \
        merry_mutex_unlock(lock);                  \
    } while (0);

#else

#define merry_logger_init()
#define merry_logger_close()
#define merry_log(_device_, _info_, _details_)
#define merry_llog(_device_, _info_, _msg_, ...)

#endif

#define merry_init_logger() merry_logger_init()
#define merry_close_logger() merry_logger_close()
#define _log_(_device_, _info_, _details_) merry_log(_device_, _info_, _details_)
#define _llog_(_device_, _info_, _msg_, ...) merry_llog(_device_, _info_, _msg_, __VA_ARGS__)

// definition for devices
#define _OS_ "Manager"
#define _CORE_ "Core"
#define _MEM_ "Memory"
#define _READER_ "Reader"
#define _REQHDLR_ "Request Handler"
#define _DECODER_ "Decoder"

#endif