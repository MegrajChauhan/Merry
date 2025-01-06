#ifndef _MERRY_CONSOLE_
#define _MERRY_CONSOLE_

#include <stdio.h>
#include "merry_config.h"

#define _MERRY_LOG_INFO_ 0
#define _MERRY_LOG_WARN_ 1
#define _MERRY_LOG_ERR_ 2

#define _MERRY_DBG_

#define nil(ptr) (ptr = NULL)

#ifdef _USE_LINUX_
#define RESET "\033[0m"
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

#define BOLDBLACK "\033[1;30m"
#define BOLDRED "\033[1;31m"
#define BOLDGREEN "\033[1;32m"
#define BOLDYELLOW "\033[1;33m"
#define BOLDBLUE "\033[1;34m"
#define BOLDMAGENTA "\033[1;35m"
#define BOLDCYAN "\033[1;36m"
#define BOLDWHITE "\033[1;37m"

#define UNDERLINE "\033[4m"
#define BOLD "\033[1m"

#else
// for windows as such
#endif

// For any errors from the VM(not a debugging tool but to be used properly)
#define mreport(msg) fprintf(stderr, BOLDCYAN "REPORT: " msg RESET "\n")
#define mreportA(msg, ...) fprintf(stderr, BOLDCYAN "REPORT: " msg RESET "\n", __VA_ARGS__)

#ifdef _MERRY_DBG_

// Log any error
#define err_log(msg, ...) fprintf(stderr, BOLDRED "ERROR: " RESET msg "\n", __VA_ARGS__)

// Log any error(no arguments(or causes))
#define inerr_log(msg) err_log(msg, NULL)

// Log any message
#define log(msg, ...) fprintf(stdout, BOLDGREEN "LOG: " RESET msg "\n", __VA_ARGS__)

// Log any message(with no arguments)
#define inlog(msg) log(msg, NULL)

#define merry_log(level, msg, ...)                                                \
    do                                                                            \
    {                                                                             \
        switch (level)                                                            \
        {                                                                         \
        case _MERRY_LOG_INFO_:                                                    \
            log(msg, __VA_ARGS__);                                                \
            break;                                                                \
        case _MERRY_LOG_WARN_:                                                    \
            fprintf(stderr, "%sWARN:%s " msg "\n", BOLDBLUE, RESET, __VA_ARGS__); \
            break;                                                                \
        case _MERRY_LOG_ERR_:                                                     \
            fprintf(stderr, "%sERROR:%s " msg "\n", BOLDRED, RESET, __VA_ARGS__); \
            break;                                                                \
        default:                                                                  \
            break;                                                                \
        }                                                                         \
    } while (0)

#define mcrash() exit(-1)
#define massert(ptr)                                                                                                                \
    do                                                                                                                              \
    {                                                                                                                               \
        if (!ptr)                                                                                                                   \
        {                                                                                                                           \
            merry_log(_MERRY_LOG_ERR_, "Pointer assertion failed [Line %d : File %s'%s'%s]", __LINE__, BOLDWHITE, __FILE__, RESET); \
        }                                                                                                                           \
    } while (0)

#define massert_field(ptr, field) \
    do                            \
    {                             \
        massert(ptr);             \
        massert(ptr->field);      \
    } while (0)

#define massertion(condition)                                                                                                               \
    do                                                                                                                                      \
    {                                                                                                                                       \
        if (!condition)                                                                                                                     \
            merry_log(_MERRY_LOG_ERR_, "Assertion Failed: " #condition "[LINE %d in FILE %s'%s'%s]", __LINE__, BOLDWHITE, __FILE__, RESET); \
    } while (0)

#else
#define err_log(msg, ...)
#define inerr_log(msg)
#define log(msg, ...)
#define inlog(msg)
#define merry_log(level, msg, ...)
#define mcrash()
#define massert(ptr)
#define massert_field(ptr, field)
#endif

#endif