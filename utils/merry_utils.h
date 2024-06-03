/*
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
#ifndef _MERRY_UTILS_
#define _MERRY_UTILS_

#define _stringify(x) #x
#define _glued(x, y) x##y
#define _toggle(x) (~x) >> 63
#define _MERRY_STRINGIFY_(to_str) _stringify(to_str)
#define _MERRY_CONCAT_(x, y) _glued(x, y)
#define _MERRY_TOGGLE_(x) _toggle(x) // toggle a boolean value

#define _MERRY_ALIGN_MAGIC_NUM_ 0xFFFFFFFFFFFFFFF8

#define merry_align_size(size) (size + 7) & _MERRY_ALIGN_MAGIC_NUM_

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

#endif