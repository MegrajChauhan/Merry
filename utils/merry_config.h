/*
 * Configuration for the Merry VM
 * MIT License
 *
 * Copyright (c) 2023 MegrajChauhan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _MERRY_CONFIGS_
#define _MERRY_CONFIGS_

#define _MERRY_LITTLE_ENDIAN_ 0x01
#define _MERRY_BIG_ENDIAN_ 0x02

/*
 * Check for the endianness of the system. We can use this to our advantage for making the VM faster.
 */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _MERRY_BYTE_ORDER_ _MERRY_LITTLE_ENDIAN_
#else
#define _MERRY_BYTE_ORDER_ _MERRY_BIG_ENDIAN_
#endif

#if __SIZEOF_POINTER__ != 8
#error Merry requires 64-bit host machines to run. Host machine is not 64-bit.
#endif

/*compiler detection*/
#if defined(__cplusplus)
#define _MERRY_COMPILER_CPP_ 1 // indicating that the compiler is c++
#endif

/*For C++ compilers*/
#if defined(_MERRY_COMPILER_CPP_)
#define _MERRY_EXTERN_HEADER_ \
    extern "C"                \
    {
#define _MERRY_EXTERN_HEADER_E_ }
#else
#define _MERRY_EXTERN_HEADER_
#define _MERRY_EXTERN_HEADER_E_
#endif

/*for now we only check for amd architecture*/
#if defined(__amd64) || defined(__amd64__)
// the CPU is an AMD CPU
#define _MERRY_HOST_CPU_AMD_ 1
#endif

// for only x86_64 available
#if defined(_MERRY_HOST_CPU_AMD_)
// the CPU supports x86_64 instructions[can be set by intel processors as well]
#define _ASP_HOST_CPU_x86_64_ARCH_ 1
#endif

// for now we only do this for the linux systems because we don't know anything about other systems
#if defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
// the os is linux
#define _ASP_HOST_OS_LINUX_ 1
#endif

#define _MERRY_TO_BOOL_(x) !!(x) // convert x to bool

#ifndef surelyT

#define surelyT(x) __builtin_expect(!!(x), 1) // tell the compiler that the expression x is most likely to be true
#define surelyF(x) __builtin_expect(!!(x), 0) // tell the compiler that the expression x is most likely to be false

#endif

/*Based on optimizations, this may be set*/
#if defined(_MERRY_OPTIMIZE_)
#define _ASP_ALWAYS_INLINE static inline __attribute__((always_inline))
#else
#define _ASP_ALWAYS_INLINE static inline
#endif

#define _MERRY_NO_DISCARD_ [[nodiscard]]
#define _MERRY_NO_THROW_ __attribute__((no_throw))
#define _MERRY_NO_RETURN_ __attribute__((no_return))
#define _MERRY_NO_NULL_ __attribute__((nonnull))

#if __SIZEOF_LONG__ == __SIZEOF_LONG_LONG__
#define _MERRY_LONG_ unsigned long
#else
#define _MERRY_LONG_ unsigned long long
#endif

#endif