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
static_assert(0, "CM:w:cm_gateway.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_GATEWAY_H
#define _CLANG_CM_GATEWAY_H

#include "cm_send.h"
#include "cm_util.h"


// API for access to gateway hardware (e.g. thread sync monitors)

typedef enum _SFID_GW {
  SFID_GATEWAY = 0x03
} SFID_GW;

typedef enum _SubFuncID {
  SUBID_RSVD_1 = 0x0,
  SUBID_GW_SIGNAL_EVENT = 0x01,
  SUBID_GW_MONITOR_EVENT = 0x02,
  SUBID_GW_MONITOR_NO_EVENT = 0x03,
  SUBID_GW_BARRIER = 0x04,
  SUBID_RSVD_2 = 0x05,
  SUBID_GW_WAIT_EVENT = 0x06,
  SUBID_RSVD_3 = 0x07
} SubFuncID;

#if defined(CM_GEN7_5) || defined(CM_GEN8) || defined(CM_GEN8_5) ||            \
    defined(CM_GEN9) || defined(CM_GEN9_5) || defined(CM_GEN10) ||             \
    !defined(CM_GENX)

#define signal_event(...)                                                      \
  CM_STATIC_ERROR(0, "signal_event is only supported for ICL+. Ensure "        \
                     "compile flags reflect this.");
#define monitor_event(...)                                                     \
  CM_STATIC_ERROR(0, "monitor_event is only supported for ICL+. Ensure "       \
                     "compile flags reflect this.");
#define monitor_no_event(...)                                                  \
  CM_STATIC_ERROR(0, "monitor_no_event is only supported for ICL+. Ensure "    \
                     "compile flags reflect this.");
#define wait_event(...)                                                        \
  CM_STATIC_ERROR(0, "wait_event is only supported for ICL+. Ensure compile "  \
                     "flags reflect this.");

/// \brief Wrapper function for cm_wait builtin
///
/// \param mask 8 bit mask for which thread dependency to honour - if 1 then the
/// dependency is ignored and the thread will not wait
///
/// This is actually a wrapper function for the call to the internal builtin
/// (which will be overridden for later gen variants using the software
/// scoreboard implementation)
CM_INLINE void cm_wait(unsigned char mask = 0) { __cm_builtin_cm_wait(mask); }

/// \brief Wrapper function for cm_signal - this is a no-op for < gen11
///
CM_INLINE void cm_signal() {}

template <int NumDeps>
CM_INLINE void cm_signal() {
  constexpr unsigned N = details::getNextPowerOf2(NumDeps);
  CM_STATIC_ERROR(N <= 8, "the maximal number dependencies cannot exceed 8");
}

#else

/// \brief Hardware Thread Monitor signal event
///
/// \param event_id 24 bit value for the event id (extra bits not checked)
///
/// Code is generated to perform the gateway operation "signal event" using the
/// event_id passed in as the identifier. Only the bottom 24 bits are valid and
/// any others are ignored
///
CM_INLINE void signal_event(unsigned int event_id) {
  matrix<ushort, 1, 16> payload = 0;
  unsigned msgLength = 1;
  unsigned rspLength = 0;
  uint msgDesc = (SUBID_GW_SIGNAL_EVENT & 0x7ffff) + (1 << 19) +
                 ((rspLength & 0x1f) << 20) + ((msgLength & 0xf) << 25);
  int dummy = 0;

  payload.format<uint>()[0] = event_id;

  cm_send(dummy, payload, SFID_GATEWAY, msgDesc, 0u /* sendc */);
}

/// \brief Hardware Thread Monitor monitor_event event
///
/// \param event_id 24 bit value for the event id (extra bits not checked)
///
/// Code is generated to perform the gateway operation "monitor event" using the
/// event_id passed in as the identifier. Only the bottom 24 bits are valid and
/// any others are ignored. Monitor event tells the hardware to include this
/// thread in notification for changes to the specified event. (This is done
/// through the wait call - wait with no monitor will cause immediate return
/// with a value of 0)
///
CM_INLINE void monitor_event(unsigned int event_id) {
  matrix<ushort, 1, 16> payload = 0;
  unsigned msgLength = 1;
  unsigned rspLength = 0;
  uint msgDesc = (SUBID_GW_MONITOR_EVENT & 0x7ffff) + (1 << 19) +
                 ((rspLength & 0x1f) << 20) + ((msgLength & 0xf) << 25);
  int dummy = 0;

  payload.format<uint>()[0] = event_id;

  cm_send(dummy, payload, SFID_GATEWAY, msgDesc, 0u /* sendc */);
}

/// \brief Hardware Thread Monitor monitor_no event
///
///
/// Code is generated to perform the gateway operation "monitor no event".
/// This clears any events currently being monitored
///

CM_INLINE void monitor_no_event(void) {
  matrix<ushort, 1, 16> payload = 0;
  unsigned msgLength = 1;
  unsigned rspLength = 0;
  uint msgDesc = (SUBID_GW_MONITOR_NO_EVENT & 0x7ffff) + (1 << 19) +
                 ((rspLength & 0x1f) << 20) + ((msgLength & 0xf) << 25);
  int dummy = 0;
  cm_send(dummy, payload, SFID_GATEWAY, msgDesc, 0u /* sendc */);
}

/// \brief Hardware Thread Monitor wait event
///
/// \param timer_value 10-bit value for the event id (should be compile time
/// constant). A value of 0 causes an indefinite wait (i.e. no timeout)
///
/// Code is generated to perform the gateway operation "wait" using the
/// timer_value passed in as the timeout. Only the bottom 10 bits are valid.
/// Only one event may be monitored/waited on at a time
///
CM_INLINE unsigned int wait_event(unsigned short timer_value) {
  matrix<ushort, 1, 16> payload = 0;
  matrix<ushort, 1, 16> response = 0;
  unsigned msgLength = 1;
  unsigned rspLength = 1;
  uint msgDesc = (SUBID_GW_WAIT_EVENT & 0x7ffff) + (1 << 19) +
                 ((rspLength & 0x1f) << 20) + ((msgLength & 0xf) << 25);
  int dummy = 0;

  payload.format<unsigned short>()[0] = timer_value;

  cm_send(response, payload, SFID_GATEWAY, msgDesc, 0u /* sendc */);
  unsigned int lock = response.format<unsigned int>()[0];
  __cm_builtin_dummy_mov(response.format<unsigned short>()[0]);

  return lock;
}

/// \brief Wrapper function for cm_wait builtin
///
/// \param mask 8 bit mask for which thread dependency to honour - if 1 then the
/// dependency is ignored and the thread will not wait
///
/// This is actually a wrapper function for the call to the internal builtin
/// (which will be overridden for later gen variants using the software
/// scoreboard implementation)
CM_INLINE void cm_wait(unsigned char mask = 0xff) {
  SurfaceIndex BoardIdx = cm_scoreboard_bti();
  matrix<uint, 1, 1> scoreboard_entry_value = 0;
  int x1 = get_thread_origin_x();
  int y1 = get_thread_origin_y();
  int EventID = (y1 << 10) | x1;
  monitor_event(EventID);
  read(BoardIdx, x1 * 4, y1, scoreboard_entry_value); // offset is in bytes
  while ((scoreboard_entry_value[0][0] & mask) != 0) {
    wait_event(EventID);
    read(BoardIdx, x1 * 4, y1, scoreboard_entry_value);
  }

  monitor_no_event();
}

/// \brief Wrapper function for cm_signal - this is a no-op for < gen11
///
CM_INLINE void cm_signal() {
  int x1 = get_thread_origin_x();
  int y1 = get_thread_origin_y();

  SurfaceIndex BoardIdx = cm_scoreboard_bti();

  vector<uchar, 8> Signals_InBytes;
  Signals_InBytes.format<U32>()(0) = 0xF7FBFDFE;
  Signals_InBytes.format<U32>()(1) = 0x7FBFDFEF;
  vector<int, 8> Signals = Signals_InBytes;
  vector<char, 16> depVec = cm_scoreboard_deltas();

  // TODO: implement a runtime dependency count.
  constexpr unsigned N = 8;
  vector<uint, N> ThreadSpace_X = x1 - depVec.select<N, 1>(0);
  vector<uint, N> ThreadSpace_Y = y1 - depVec.select<N, 1>(8);

  vector<int, N> res;
  write_typed_atomic<ATOMIC_AND>(BoardIdx, res, Signals.select<N, 1>(),
                                 ThreadSpace_X,
                                 ThreadSpace_Y); // for 2d atomic

  #pragma unroll
  for (unsigned i = 0; i < N; i++) {
    unsigned EventID = (ThreadSpace_Y[i] << 10) | ThreadSpace_X[i];
    signal_event(EventID);
  }
}

/// This is more efficient than the one without specifing the maximal number
/// of depedencies.
template <int NumDeps>
CM_INLINE void cm_signal() {
  constexpr unsigned N = details::getNextPowerOf2(NumDeps);
  CM_STATIC_ERROR(N <= 8, "the maximal number dependencies cannot exceed 8");

  int x1 = get_thread_origin_x();
  int y1 = get_thread_origin_y();

  SurfaceIndex BoardIdx = cm_scoreboard_bti();

  vector<uchar, 8> Signals_InBytes;
  Signals_InBytes.format<U32>()(0) = 0xF7FBFDFE;
  Signals_InBytes.format<U32>()(1) = 0x7FBFDFEF;
  vector<int, 8> Signals = Signals_InBytes;
  vector<char, 16> depVec = cm_scoreboard_deltas();

  vector<uint, N> ThreadSpace_X = x1 - depVec.select<N, 1>(0);
  vector<uint, N> ThreadSpace_Y = y1 - depVec.select<N, 1>(8);

  vector<int, N> res;
  write_typed_atomic<ATOMIC_AND>(BoardIdx, res, Signals.select<N, 1>(),
                                 ThreadSpace_X,
                                 ThreadSpace_Y); // for 2d atomic

  #pragma unroll
  for (unsigned i = 0; i < N; i++) {
    unsigned EventID = (ThreadSpace_Y[i] << 10) | ThreadSpace_X[i];
    signal_event(EventID);
  }
}
#endif

#endif /* _CLANG_CM_GATEWAY_H */
