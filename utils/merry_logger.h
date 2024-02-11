#ifndef _MERRY_LOGGER_
#define _MERRY_LOGGER_

#include <stdio.h>
#include <stdarg.h>
#include "../sys/merry_thread.h"

// #define _MERRY_LOGGER_ENABLED_

struct MerryLogger
{
    FILE *f;
    mbool_t enabled; // enable debugger? creates a file named "merry.log"
    MerryMutex *lock;
};

static struct MerryLogger logger;

mret_t merry_logger_init(mbool_t enable_flag);

void merry_logger_close();

// The logging format is: [Device][Additional Information]: Details
void merry_log(mstr_t _device_, mstr_t _info_, mstr_t _details_);

void merry_llog(mstr_t _device_, mstr_t _info_, mstr_t _msg_, ...);

// definition for devices
#define _OS_ "Manager"
#define _CORE_ "Core"
#define _MEM_ "Memory"
#define _READER_ "Reader"
#define _REQHDLR_ "Request Handler"
#define _DECODER_ "Decoder"

// #define _INFO_INIT_ "Intialization"
// #define _INFO_EXEC_ "Executing"

#endif