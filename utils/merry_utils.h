#ifndef _MERRY_UTILS_
#define _MERRY_UTILS_

#include <stdio.h>
#include <stdlib.h>

#define _stringify(x) #x
#define _glued(x, y) x##y
#define _toggle(x) (~x) >> 63
#define _MERRY_STRINGIFY_(to_str) _stringify(to_str)
#define _MERRY_CONCAT_(x, y) _glued(x, y)
#define _MERRY_TOGGLE_(x) _toggle(x) // toggle a boolean value

// #define _MERRY_ALIGN_MAGIC_NUM_ 0xFFFFFFFFFFFFFFF8

// #define merry_align_size(size) (size + 7) & _MERRY_ALIGN_MAGIC_NUM_

#define _MERRY_TO_BOOL_(x) !!(x) // convert x to bool

#ifndef surelyT

#define surelyT(x) __builtin_expect(!!(x), 1) // tell the compiler that the expression x is most likely to be true
#define surelyF(x) __builtin_expect(!!(x), 0) // tell the compiler that the expression x is most likely to be false

#endif

#if defined(_MERRY_OPTIMIZE_)
#define _MERRY_ALWAYS_INLINE_ __attribute__((always_inline))
#else
#define _MERRY_ALWAYS_INLINE_ static inline
#endif

#define _MERRY_NO_DISCARD_ __attribute__((nodiscard))
#define _MERRY_NO_THROW_ __attribute__((no_throw))
#define _MERRY_NO_RETURN_ __attribute__((no_return))
#define _MERRY_NO_NULL_ __attribute__((nonnull))

#define _MERRY_INTERNAL_ static // for a variable or a function that is localized to a module only
#define _MERRY_LOCAL_ static    // any static variable inside a function

// tools
#define merry_talk(f, kind, message, ...) fprintf(f, kind ": " message, __VA_ARGS__)

#define merry_check_condition(cond)                                                                                                     \
    do                                                                                                                                  \
    {                                                                                                                                   \
        if (surelyF(!(cond)))                                                                                                           \
        {                                                                                                                               \
            merry_talk(stderr, "ERROR", "Condition: "_MERRY_STRINGIFY_((cond)) ": Failed. Line %d[FILE: %s]\n", __LINE__, __FILE__); \
            exit(-1);                                                                                                                   \
        }                                                                                                                               \
    } while (0)

#ifdef _MERRY_RELEASE_

#define merry_assert(cond)
#define merry_check_ptr(ptr)
#define merry_log(msg, ...)
#define merry_suggest(msg, ...)

#else
#define _MERRY_TEST_
#define merry_assert(cond) merry_check_condition(cond)

/**
 * The existence of this pointer checker is based on:
 * -- The VM may pass in NULL pointers or invalid pointers
 * So during the testing, if we figure out which pointers are freed and how, we can solve the issue
 * and this macro is  useless in the release mode.
 */
#define merry_check_ptr(ptr) merry_assert(ptr != NULL)

#define merry_log(msg, ...) merry_talk(stdout, "LOG", msg, __VA_ARGS__)

#define merry_suggest(msg, ...) merry_talk(stdout, "NOTE", msg, __VA_ARGS__)

#endif

#define merry_err(msg, ...) merry_talk(stderr, "ERROR", msg, __VA_ARGS__)

#define merry_msg(msg, ...) fprintf(stderr, msg "\n", __VA_ARGS__)

#endif
