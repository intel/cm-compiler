/*
 * Copyright (c) 2019, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_target.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_TARGET_H_
#define _CLANG_CM_TARGET_H_

// CM_HAS_VA is defined if BDW+ video analytics features are available
#if CM_GENX >= 800
#define CM_HAS_VA 1
#endif

// CM_HAS_VA_PLUS is defined if SKL+ video analytics features are available
#if CM_GENX >= 900
#define CM_HAS_VA_PLUS 1
#endif

// CM_HAS_LONG_LONG is defined if long long is available
#define CM_HAS_LONG_LONG 1
#if CM_GENX && (CM_GENX < 800 || CM_GENX == 940 || CM_GENX == 1150 || CM_GENX == 1250)
#undef CM_HAS_LONG_LONG
#endif

// CM_HAS_DOUBLE is defined if double is available
#define CM_HAS_DOUBLE 1
#if CM_GENX == 940 || CM_GENX == 1150 || CM_GENX == 1250
#undef CM_HAS_DOUBLE
#endif

// Make Gen target specific warnings into errors
#pragma clang diagnostic error "-Wgen-target"

#endif /* _CLANG_CM_TARGET_H_ */
