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

#ifndef CM_VS2013_H
#define CM_VS2013_H

#if _MSC_VER >= 1800
// Workaround for VS2013 type traits  - these are not supported by icl
// so we force them all to false here to allow them to appear without
// compiler errors being generated.
// Note: They should not be used as they are not actually implemented!
#define __is_nothrow_assignable(X,...) (false)
#define __is_nothrow_constructible(X,...) (false)
#define __is_nothrow_destructible(X,...) (false)
#define __is_constructible(X,...) (false)
#define __is_destructible(X,...) (false)
#define __is_trivially_assignable(X,...) (false)
#define __is_trivially_constructible(X,...) (false)
#endif

#endif /* CM_VS2013_H */
