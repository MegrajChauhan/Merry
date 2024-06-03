#ifndef _MERRY_CONSOLE_
#define _MERRY_CONSOLE_

#include <stdio.h>

#define err_log(msg, ...) fprintf(stderr, msg, __VA_ARGS__)
#define inerr_log(msg) err_log(msg, "")

#define log(msg, ...) fprintf(stdout, msg, __VA_ARGS__)
#define inlog(msg) log(msg, "")

#define _MERRY_LOG_INFO_ 0
#define _MERRY_LOG_WARN_ 1
#define _MERRY_LOG_ERR_ 2

#define merry_log(level, msg, ...) \
    do {                           \
        switch (level) {           \
            case _MERRY_LOG_INFO_: \
                log(msg, __VA_ARGS__); break; \
            case _MERRY_LOG_WARN_: \
                fprintf(stderr, "\033[93mWARN:\033[0m " msg, __VA_ARGS__); break; \
            case _MERRY_LOG_ERR_: \
                fprintf(stderr, "\033[91mERROR:\033[0m " msg, __VA_ARGS__); break; \
            default: break; \
        } \
    } while (0)


#endif