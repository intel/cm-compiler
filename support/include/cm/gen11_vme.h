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
static_assert(0, "CM:w:gen11_vme.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_gen11_VME_H_
#define _CLANG_gen11_VME_H_

#include "cm_send.h"
#include "cm_util.h"
#include "gen9_vme.h" // Included to obtain legacy functionality

typedef enum _VMEMsgTypeHEVC_ {
  __VME_HEVC_SKIP_RDE_MSG = 0,
  __VME_HEVC_IME_MSG = 1,
  __VME_HEVC_INTER_RDE_MSG = 2,
  __VME_HEVC_INTRA_RDE_MSG = 3,
  __VME_HEVC_FME_MESH_MSG = 4,
} __VMEMsgTypeHEVC;

// Gen11 HEVC-specific functions

// Macros for forming descriptor values for cm_send() and cm_sends()
// The macros use a functional style - they take a descriptor value and any
// required additional parameters and return an appropriate new descriptor
// value

// Set number of inputs (0..15)
#define SEND_SET_Descriptor_Num_Inputs(v, i) ((v) | (((i) & 0xf) << 25))
// Set number of inputs (0..31) in Extended Descriptor
#define SEND_SET_Descriptor_Num_InputsExt(v, i) ((v) | (((i) & 0x1f) << 6))
// Set number of outputs (0..31)
#define SEND_SET_Descriptor_Num_Outputs(v, o) ((v) | (((o) & 0x1f) << 20))
#define SEND_SET_Descriptor_Header_Present(v) ((v) | (1 << 19))
#define SEND_SET_Descriptor_HEVC_Mode(v) ((v) | (1 << 18))
#define SEND_SET_Descriptor_StreamIn_Enabled(v) ((v) | (1 << 16))
// Set function, valid values in __VMEMsgTypeHEVC
#define SEND_SET_Descriptor_Function(v, f) ((v) | (((f) & 0x7) << 8))

// Create a descriptor value from a surface index (0..255)
#define SEND_INIT_Descriptor_Surface(s) (cm_get_value(s) & 0xff)

template <int N>
CM_INLINE void
run_vme_fbr(matrix<uchar, 4, 32> UNIInput, matrix<uchar, 4, 32> FBRInput,
            SurfaceIndex curSurfIndex, uchar FBRMbMode, uchar FBRSubMbShape,
            uchar FBRSubPredMode, matrix_ref<uchar, N, 32> FBROutput) {
  matrix<uint, 8, 8> src = 0;
  // mov(96)    VX(0, 0)<1>, UNIInput
  src.format<uint>().select<32, 1>(0) =
      UNIInput.format<uint>().select<32, 1>(0);
  // mov  (128)   VX(3,0)<1>,  FBRInput
  src.format<uint>().select<32, 1>(32) =
      FBRInput.format<uint>().select<32, 1>(0);
  // mov  (1)     VX(2,20)<1>, FBRMbMode
  src.format<uchar, 8, 32>()(2, 20) = FBRMbMode;
  // mov  (1)     VX(2,21)<1>, FBRSubMbShape
  src.format<uchar, 8, 32>()(2, 21) = FBRSubMbShape;
  //  mov  (1)     VX(2,22)<1>, FBRSubPredMode
  src.format<uchar, 8, 32>()(2, 22) = FBRSubPredMode;

  unsigned fCtrl = 0x3 << 13;
  unsigned rspLength = N; // used to be 7 in previous platforms
  unsigned msglength = 8;
  uint Descriptor = (fCtrl & 0x7FFFF) + (1 << 19) + ((rspLength & 0x1f) << 20) +
                    ((msglength & 0xF) << 25);

  Descriptor = cm_get_value(curSurfIndex) + Descriptor;
  cm_send(FBROutput.format<ushort, N, 16>(), src.format<ushort, 8, 16>(),
          __SFID_CRE, Descriptor, 0u);
}

template <int N>
CM_INLINE void
run_vme_sic(matrix<uchar, 4, 32> UNIInput, matrix<uchar, 4, 32> SICInput,
            SurfaceIndex curSurfIndex, matrix_ref<uchar, N, 32> UNIOutput) {
  matrix<uint, 8, 8> src = 0;
  // mov  (96)    VX(0,0)<1>,  UNIInput
  src.format<uint>().select<32, 1>(0) =
      UNIInput.format<uint>().select<32, 1>(0);
  // mov  (128)   VX(3,0)<1>,  SICInput
  src.format<uint>().select<32, 1>(32) =
      SICInput.format<uint>().select<32, 1>(0);
  unsigned fCtrl = 0x1 << 13;
  unsigned rspLength = N; // used to be 7 in the previous platforms
  unsigned msgLength = 8;
  uint Descriptor = (fCtrl & 0x7FFFF) + (1 << 19) + ((rspLength & 0x1f) << 20) +
                    ((msgLength & 0xF) << 25);

  Descriptor = cm_get_value(curSurfIndex) + Descriptor;
  cm_send(UNIOutput.format<ushort, N, 16>(), src.format<ushort, 8, 16>(),
          __SFID_CRE, Descriptor, 0u);
}

template <int N>
CM_INLINE void
run_vme_idm(matrix<uchar, 4, 32> UNIInput,
            SurfaceIndex curSurfIndex, matrix_ref<uchar, N, 32> IDMOutput) {
  unsigned fCtrl = 0;
  unsigned rspLength = N; // used to 16 in the previous platforms
  unsigned msglength = 4;
  uint Descriptor = (fCtrl & 0x7FFFF) + (1 << 19) + ((rspLength & 0x1f) << 20) +
                    ((msglength & 0xF) << 25);

  Descriptor = cm_get_value(curSurfIndex) + Descriptor;
  cm_send(IDMOutput.format<ushort, N, 16>(), UNIInput.format<ushort, 4, 16>(),
          __SFID_VME, Descriptor, 0u);
}

// Gen11 HEVC VME support

#define selrows(r) select<r, 1, 32, 1>(0, 0).format<ushort, r, 16>()

template <typename Dummy = void>
CM_INLINE void
cm_vme_hevc_skip_rde_implicit(/*const*/ matrix_ref<uchar, 10, 32> RDEInput,
                              SurfaceIndex curSurfIndex,
                              matrix_ref<uchar, 4, 32> RDEOutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, RDEInput.n_rows());
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor =
      SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_SKIP_RDE_MSG);
  Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, RDEOutput.n_rows());

  cm_send(RDEOutput.selrows(4), RDEInput.selrows(10), __SFID_CRE, Descriptor,
          0);
}

template <typename Dummy = void>
CM_INLINE void
cm_vme_hevc_skip_rde_explicit(/*const*/ matrix_ref<uchar, 6, 32> RDEInput,
                              SurfaceIndex curSurfIndex,
                              matrix_ref<uchar, 12, 32> RDEOutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, RDEInput.n_rows());
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor =
      SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_SKIP_RDE_MSG);
  Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, RDEOutput.n_rows());

  cm_send(RDEOutput.selrows(12), RDEInput.selrows(6), __SFID_CRE, Descriptor,
          0);
}

template <typename Dummy = void>
CM_INLINE void cm_vme_hevc_ime(/*const*/ matrix_ref<uchar, 5, 32> IMEInput,
                               /*const*/ matrix_ref<uchar, 10, 32> StreamInput,
                               int LengthStreamInput, SurfaceIndex curSurfIndex,
                               matrix_ref<uchar, 11, 32> StreamOutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, IMEInput.n_rows());
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor = SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_IME_MSG);
  Descriptor =
      SEND_SET_Descriptor_Num_Outputs(Descriptor, StreamOutput.n_rows());

  if (LengthStreamInput == 10) {
    Descriptor = SEND_SET_Descriptor_StreamIn_Enabled(Descriptor);
    U32 ExDescriptor =
        SEND_SET_Descriptor_Num_InputsExt(__SFID_VME, StreamInput.n_rows());

    cm_sends(StreamOutput.selrows(11), IMEInput.selrows(5),
             StreamInput.selrows(10), ExDescriptor, Descriptor, 0);
  } else
    cm_send(StreamOutput.selrows(11), IMEInput.selrows(5), __SFID_VME,
            Descriptor, 0);
}

template <typename Dummy = void>
CM_INLINE void
cm_vme_hevc_inter_rde_implicit(/*const*/ matrix_ref<uchar, 10, 32> RDEInput,
                               SurfaceIndex curSurfIndex,
                               matrix_ref<uchar, 4, 32> RDEOutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, RDEInput.n_rows());
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor =
      SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_INTER_RDE_MSG);
  Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, RDEOutput.n_rows());

  cm_send(RDEOutput.selrows(4), RDEInput.selrows(10), __SFID_CRE, Descriptor,
          0);
}

template <typename Dummy = void>
CM_INLINE void
cm_vme_hevc_inter_rde_explicit(/*const*/ matrix_ref<uchar, 5, 32> RDEInput,
                               SurfaceIndex curSurfIndex,
                               matrix_ref<uchar, 4, 32> RDEOutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, RDEInput.n_rows());
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor =
      SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_INTER_RDE_MSG);
  Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, RDEOutput.n_rows());

  cm_send(RDEOutput.selrows(4), RDEInput.selrows(5), __SFID_CRE, Descriptor, 0);
}

template <typename Dummy = void>
CM_INLINE void
cm_vme_hevc_intra_rde(/*const*/ matrix_ref<uchar, 14, 32> RDEInput,
                      int LengthLCNPInput, SurfaceIndex curSurfIndex,
                      matrix_ref<uchar, 4, 32> RDEOutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, 4 + LengthLCNPInput);
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor =
      SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_INTRA_RDE_MSG);
  Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, RDEOutput.n_rows());

  cm_send(RDEOutput.selrows(4), RDEInput.selrows(14), __SFID_CRE, Descriptor,
          0);
}

template <typename Dummy = void>
CM_INLINE void cm_vme_hevc_fme_mesh(/*const*/ matrix_ref<uchar, 4, 32> FMEInput,
                                    SurfaceIndex curSurfIndex,
                                    matrix_ref<uchar, 3, 32> FMEOutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, FMEInput.n_rows());
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor =
      SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_FME_MESH_MSG);
  Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, FMEOutput.n_rows());

  cm_send(FMEOutput.selrows(3), FMEInput.selrows(4), __SFID_CRE, Descriptor, 0);
}

#endif /* _CLANG_gen11_VME_H_ */
