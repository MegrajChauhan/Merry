#ifndef _MERRY_LOGGER_
#define _MERRY_LOGGER_

#include <stdio.h>
#include <stdarg.h>
#include "../sys/merry_thread.h"

// #define _MERRY_LOGGER_ENABLED_

#if defined(_MERRY_LOGGER_ENABLED_)
struct MerryLogger
{
    FILE *f;
    MerryMutex *lock;
};

static struct MerryLogger logger;

// we expect that the fopen and mutex init doesn't fail
void merry_logger_init();

void merry_logger_close();

// The logging format is: [Device][Additional Information]: Details
void merry_log(mstr_t _device_, mstr_t _info_, mstr_t _details_);

void merry_llog(mstr_t _device_, mstr_t _info_, mstr_t _msg_, ...);

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