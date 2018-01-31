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
static_assert(0, "CM:w:cm_gateway.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_GATEWAY_H
#define _CLANG_CM_GATEWAY_H

#include "cm_send.h"



/// \brief Wrapper function for cm_wait builtin
///
/// \param mask 8 bit mask for which thread dependency to honour - if 1 then the
/// dependency is ignored and the thread will not wait
///
/// This is actually a wrapper function for the call to the internal builtin
/// (which will be overridden for later gen variants using the software
/// scoreboard implementation)
CM_INLINE void cm_wait(unsigned char mask = 0) { __cm_builtin_cm_wait(mask); }

/// \brief Wrapper function for cm_signal
///
CM_INLINE void cm_signal(void) {}


#endif /* _CLANG_CM_GATEWAY_H */
