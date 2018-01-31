/*
 * Copyright (c) 2018, Intel Corporation
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
static_assert(0, "CM:w:cm_printf.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CM_PRINTF_H_
#define _CM_PRINTF_H_

#define printf(...) __cm_builtin_cm_printf(details::__cm_intrinsic_impl_predefined_surface(2), __VA_ARGS__)
#define cm_printf(...) __cm_builtin_cm_printf(details::__cm_intrinsic_impl_predefined_surface(2), __VA_ARGS__)

#define fprintf(BTID, ...) __cm_builtin_cm_printf(BTID, __VA_ARGS__)
#define cm_fprintf(BTID, ...) __cm_builtin_cm_printf(BTID, __VA_ARGS__)

#endif /* _CM_PRINTF_H_ */
