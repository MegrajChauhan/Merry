/*
 * Configuration for the Merry VM
 * MIT License
 *
 * Copyright (c) 2024 MegrajChauhan
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

#define _MERRY_LITTLE_ENDIAN_ 0x00
#define _MERRY_BIG_ENDIAN_ 0x01

#define _MERRY_OPTIMIZE_

#define _MERRY_VERSION_CHANGE_ 12
#define _MERRY_VERSION_MINOR_ 0
#define _MERRY_VERSION_MAJOR_ 0

#define _MERRY_VERSION_ (_MERRY_VERSION_MAJOR_ * 100000) + (_MERRY_VERSION_MINOR_ * 10000) + _MERRY_VERSION_CHANGE_
#define _MERRY_VERSION_STATE_ "test-3.7"

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
#ifndef __USE_MISC
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