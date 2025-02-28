#ifndef _MERRY_CONFIGS_
#define _MERRY_CONFIGS_

// Only 64-bit support
#if __SIZEOF_VOID__ == 4
#error Merry Only supports 64-bit systems
#endif

#define _MERRY_LITTLE_ENDIAN_ 0x00
#define _MERRY_BIG_ENDIAN_ 0x01

#define _MERRY_OPTIMIZE_

#define _MERRY_VERSION_CHANGE_ 1
#define _MERRY_VERSION_MINOR_ 1
#define _MERRY_VERSION_MAJOR_ 0

// #define _MERRY_VERSION_ (_MERRY_VERSION_MAJOR_ * 100000) + (_MERRY_VERSION_MINOR_ * 10000) + _MERRY_VERSION_CHANGE_
// #define _MERRY_VERSION_STATE_ "test-3.20"

// #define _MERRY_ID_FILE_ "__num__"               // update the location
// #define _MERRY_CMAIN_ "./build/cmain"           // update the location
// #define _MERRY_SUBSYSMAIN_ "./build/subsysmain" // update the location
#define _MERRY_VMMAIN_ "./build/mvm" // update the location

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _MERRY_BYTE_ORDER_ _MERRY_LITTLE_ENDIAN_
#else
#define _MERRY_BYTE_ORDER_ _MERRY_BIG_ENDIAN_
#endif

// Merry's endianness
#define _MERRY_ENDIANNESS_ _MERRY_BYTE_ORDER_

// Host CPU detection
#if defined(__amd64) || defined(__amd64__)
#define _MERRY_HOST_CPU_AMD_ 1
#define _MERRY_HOST_ID_ARCH_ 0x00
#endif

// Architecture detection
#if defined(__x86_64)
#define _MERRY_HOST_CPU_x86_64_ARCH_ 1
#endif

// Host OS detection
#if defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
#define _MERRY_HOST_OS_LINUX_ 1
#define _USE_LINUX_
#define _MERRY_HOST_ID_OS_ 0x00
#ifndef __USE_MISC // for STDLIB
#define __USE_MISC
#endif
#endif

#if defined(_WIN64)
#define _MERRY_HOST_OS_WINDOWS_ 1
#define _USE_WIN_
#define _MERRY_HOST_ID_OS_ 0x01
#endif

#if __SIZEOF_LONG__ == __SIZEOF_LONG_LONG__
#define _MERRY_LONG_ long
#else
#define _MERRY_LONG_ long long
#endif

#endif