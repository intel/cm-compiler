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

#pragma once
#ifndef _CLANG_CM_EMU_H_
#define _CLANG_CM_EMU_H_

#include "cm_common.h"
#include "cm_internal.h"

namespace details {

CM_NODEBUG CM_INLINE
float __impl_hex2float(uint32_t val) {
  vector<uint32_t, 1> su = val;
  vector<float, 1> sf = su.format<float>();
  return sf(0);
}

template <int N>
CM_NODEBUG CM_INLINE
vector<int16_t, N> __impl_divrem(vector<int16_t, N> ia, vector<int16_t, N> ib,
                                 vector_ref<int16_t, N> ir) {
  // Exact conversions: 16-bit -> float
  vector<float, N> fa = ia;
  vector<float, N> fb = ib;
  vector<float, N> fy = 1.0f / fb;

  // s = 1 + 2^(-20)
  vector<float, N> sf = __impl_hex2float(0x3f800008U);

  // a * (1 + 2^(-20))
  fa = fa * sf;

  // a * (1 + 2^(-20)) * fy
  vector<float, N> fq = fa * fy;

  // quotient: truncate to signed 16-bit integer
  vector<int16_t, N> iq = fq;

  // remainder
  ir = ia - iq * ib;

  // return the quotient.
  return iq;
}

template <int N>
CM_NODEBUG CM_INLINE
vector<int8_t, N> __impl_divrem(vector<int8_t, N> ia, vector<int8_t, N> ib,
                                vector_ref<int8_t, N> ir) {
  vector<int16_t, N> _ia = ia;
  vector<int16_t, N> _ib = ib;
  vector<int16_t, N> _ir;
  vector<int16_t, N> _iq = __impl_divrem(_ia, _ib, _ir);
  ir = _ir;
  return _iq;
}

// RTZ rounding mode is needed for some of the steps
// It is fine to use RTZ mode throughout the FP computation
template <int N>
CM_NODEBUG CM_INLINE
vector<int32_t, N> __impl_divrem(vector<int32_t, N> sla, vector<int32_t, N> slb,
                                 vector_ref<uint32_t, N> prem) {
  vector<float, N> ah, al, bh, bl;
  vector<float, N> y, Qh, Ql, Rh, Rl;

  vector<uint32_t, N> la_h, lb_h, la_l, lb_l, lQh, lQl, lQ, lR;
  vector<uint32_t, N> la, lb, corr_mask;
  vector<int32_t, N> sgn_a, sgn_b, sgn_q;

  // get signs and |sla|, |slb|
  sgn_a = sla >> 31;
  sgn_b = slb >> 31;
  la = (sla + sgn_a) ^ sgn_a;
  lb = (slb + sgn_b) ^ sgn_b;

  // uint32 -> single precision convert, with truncation (RZ mode)
  bh = lb;

  // convert back to uint32, to get low part
  lb_h = bh;
  lb_l = lb - lb_h;

  // low part of input, in single precision
  bl = lb_l;

  // uint32 -> single precision convert, with truncation (RZ mode)
  ah = la;

  // convert back to uint32, to get low part
  la_h = ah;
  la_l = la - la_h;

  // low part of input, in single precision
  al = la_l;

  // y = RCP(bh)
  y = 1.0f / bh;

  // y = y*(1 - 3*2^(-23))
  // RZ mode not required, used for convenience
  vector<float, N> sf = __impl_hex2float(0xb4c00000u);
  y += sf * y;

  // Qh = ah*y
  Qh = ah * y;

  // Qh = (unsigned)Qh, with truncation
  lQh = Qh;

  // convert lQh back to SP, any rounding mode is fine
  Qh = lQh;

  // ah - bh*Qh
  Rh = ah - bh * Qh;

  // al - bl*Qh
  Rl = al - bl * Qh;

  // Ql = y * (Rh + Rl)
  Rl = Rh + Rl;
  Ql = y * Rl;

  // convert Ql to integer, with truncation
  lQl = Ql;

  // integer quotient
  lQ = lQh + lQl;
  sgn_q = sgn_a ^ sgn_b;

  // integer remainder
  lR = la - lb * lQ;

  // apply correction if needed
  // if (lR >= lb) { lQ++;  lR -= lb; }
  corr_mask.merge(0xffffffff, 0, (lR >= lb));
  lQ += (corr_mask & 1);
  lR -= (lb & corr_mask);

  // remainder
  prem = (sgn_a + lR) ^ sgn_a;
  lQ = (sgn_q + lQ) ^ sgn_q;

  return lQ;
}

template <int N>
CM_NODEBUG CM_INLINE
vector<uint16_t, N>
__impl_udivrem(vector<uint16_t, N> ua, vector<uint16_t, N> ub,
               vector_ref<uint16_t, N> ur) {
  vector<float, N> fa, fb, fy, fq;
  vector<uint16_t, N> uq;

  // exact conversions: unsigned 16-bit -> float
  fb = ub;
  fa = ua;

  fy = 1.0f / fb;

  // a*(1+2^(-20))
  fa = fa * __impl_hex2float(0x3f800008u);

  // a*(1+2^(-20))*fy
  fq = fa * fy;

  // quotient:  truncate to unsigned 16-bit integer
  uq = fq;

  // remainder
  ur = ua - uq * ub;

  return uq;
}

template <int N>
CM_NODEBUG CM_INLINE
vector<uint8_t, N>
__impl_udivrem(vector<uint8_t, N> ua, vector<uint8_t, N> ub,
               vector_ref<uint8_t, N> ur) {
  vector<uint16_t, N> _ua = ua;
  vector<uint16_t, N> _ub = ub;
  vector<uint16_t, N> _ur;
  vector<uint8_t, N> uq = __impl_udivrem(_ua, _ub, _ur);
  ur = _ur;
  return uq;
}

// RZ rounding mode is needed for some of the steps
// It is fine to use RZ mode throughout the FP computation
template <int N>
CM_NODEBUG CM_INLINE
vector<uint32_t, N>
__impl_udivrem(vector<uint32_t, N> la, vector<uint32_t, N> lb,
               vector_ref<uint32_t, N> prem) {
  vector<float, N> ah, al, bh, bl;
  vector<float, N> y, Qh, Ql, Rh, Rl;
  vector<uint32_t, N> la_h, lb_h, la_l, lb_l, lQh, lQl, lQ, lR, corr_mask;

  // uint32 -> single precision convert, with truncation (RZ mode)
  bh = lb;

  // convert back to uint32, to get low part
  lb_h = bh;
  lb_l = lb - lb_h;

  // low part of input, in single precision
  bl = lb_l;

  // uint32 -> single precision convert, with truncation (RZ mode)
  ah = la;

  // convert back to uint32, to get low part
  la_h = ah;
  la_l = la - la_h;

  // low part of input, in single precision
  al = la_l;

  // y = RCP(bh)
  y = 1.0f / bh;

  // y = y*(1 - 3*2^(-23))
  // RZ mode not required, used for convenience
  y += y * __impl_hex2float(0xb4c00000u);

  // Qh = ah*y
  Qh = ah * y;

  // Qh = (unsigned)Qh, with truncation
  lQh = Qh;

  // convert lQh back to SP, any rounding mode is fine
  Qh = lQh;

  // ah - bh*Qh
  Rh = ah - bh * Qh;

  // al - bl*Qh
  Rl = al - bl * Qh;

  // Ql = y*(Rh+Rl)
  Rl = Rh + Rl;
  Ql = y * Rl;

  // convert Ql to integer, with truncation
  lQl = Ql;

  // integer quotient
  lQ = lQh + lQl;

  // integer remainder
  lR = la - lb * lQ;

  // apply correction if needed
  // if (lR >= lb) { lQ++;  lR -= lb; }
  corr_mask.merge(0xffffffff, 0, (lR >= lb));
  lQ += (corr_mask & 1);
  lR -= (lb & corr_mask);

  // remainder
  prem = lR;

  return lQ;
}

} // namespace details



#endif // _CLANG_CM_EMU_H_
