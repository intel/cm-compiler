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
static_assert(0, "CM:w:cm_send.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_SEND_H_
#define _CLANG_CM_SEND_H_

#include "cm_common.h"
#include "cm_traits.h"

/// Access the thread payload register r0.
template <typename T = void> vector<uint, 8> cm_get_r0();

/// Access the thread status register sr0.
template <typename T = void> vector<uint, 4> cm_get_sr0();

template <typename T>
typename std::enable_if<std::is_same<T, SurfaceIndex>::value ||
                            std::is_same<T, SamplerIndex>::value ||
                            std::is_same<T, VmeIndex>::value,
                        uint>::type
cm_get_value(T index);

/// \brief Raw send and split send.
///
/// @rspVar the matrix that stores the message response data.
///
/// @msgVar the matrix that stores the message payload data.
///
/// @msgVar2 the matrix that stores the second part of the message payload data
/// in a split send (Gen9+).
///
/// @exDesc the extended message descriptor, which must be a compile time
/// constant.
///
/// @msgDesc the message descriptor for the send.
///
/// @sendc the flag that indicates whether sendc should be used. It must be a
/// compile time constant (0 indicates send should be used, while 1 means
/// sendc).
///
template <typename T, int N1, typename U, int N2>
typename std::enable_if<sizeof(T) * N1 % 32 == 0 &&
                        sizeof(U) * N2 % 32 == 0>::type
cm_send(vector_ref<T, N1> rspVar, vector<U, N2> msgVar, uint exDesc,
        uint msgDesc, uint sendc);

template <typename T, int N1, int N2, typename U, int N3, int N4>
typename std::enable_if<sizeof(T) * N1 * N2 % 32 == 0 &&
                        sizeof(U) * N3 * N4 % 32 == 0>::type
cm_send(matrix_ref<T, N1, N2> rspVar, matrix<U, N3, N4> msgVar, uint exDesc,
        uint msgDesc, uint sendc);

template <typename T, int N>
typename std::enable_if<sizeof(T) * N % 32 == 0>::type
cm_send(int dummy, vector<T, N> msgVar, uint exDesc, uint msgDesc, uint sendc);

template <typename T, int N1, int N2>
typename std::enable_if<sizeof(T) * N1 * N2 % 32 == 0>::type
cm_send(int dummy, matrix<T, N1, N2> msgVar, uint exDesc, uint msgDesc,
        uint sendc);

// Split send.
template <typename U1, int N1, typename U2, int N2, typename U3, int N3>
typename std::enable_if<sizeof(U1) * N1 % 32 == 0 &&
                        sizeof(U2) * N2 % 32 == 0 &&
                        sizeof(U3) * N3 % 32 == 0>::type
cm_sends(vector_ref<U1, N1> rspVar, vector<U2, N2> msgVar,
         vector<U3, N3> msg2Var, uint exDesc, uint msgDesc, uint sendc);

template <typename U1, int N1, int N2, typename U2, int N3, int N4, typename U3,
          int N5, int N6>
typename std::enable_if<sizeof(U1) * N1 * N2 % 32 == 0 &&
                        sizeof(U2) * N3 * N4 % 32 == 0 &&
                        sizeof(U3) * N5 * N6 % 32 == 0>::type
cm_sends(matrix_ref<U1, N1, N2> rspVar, matrix<U2, N3, N4> msgVar,
         matrix<U3, N5, N6> msg2Var, uint exDesc, uint msgDesc, uint sendc);

template <typename U1, int N1, typename U2, int N2>
typename std::enable_if<sizeof(U1) * N1 % 32 == 0 &&
                        sizeof(U2) * N2 % 32 == 0>::type
cm_sends(int dummy, vector<U1, N1> msgVar, vector<U2, N2> msg2Var, uint exDesc,
         uint msgDesc, uint sendc);

template <typename U1, int N1, int N2, typename U2, int N3, int N4>
typename std::enable_if<sizeof(U1) * N1 * N2 % 32 == 0 &&
                        sizeof(U2) * N3 * N4 % 32 == 0>::type
cm_sends(int dummy, matrix<U1, N1, N2> msgVar, matrix<U2, N3, N4> msg2Var,
         uint exDesc, uint msgDesc, uint sendc);

#endif
