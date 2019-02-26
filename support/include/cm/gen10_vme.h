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
static_assert(0, "CM:w:gen10_vme.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_gen10_VME_H_
#define _CLANG_gen10_VME_H_

#include "cm_send.h"
#include "cm_util.h"
#include "gen9_vme.h" // Included to obtain legacy functionality

typedef enum _VMEMsgTypeHEVC_ {
  __VME_HEVC_SIC_MSG = 0,
  __VME_HEVC_IME_MSG = 1,
  __VME_HEVC_FBR_MSG = 2,
  __VME_HEVC_HPM_MSG = 3,
  __VME_HEVC_SRM_MSG = 4,
  __VME_HEVC_RPM_MSG = 5,
} __VMEMsgTypeHEVC;

// Gen10 HEVC-specific functions

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
run_vme_idm(matrix<uchar, 4, 32> UNIInput, matrix<uchar, 1, 32> IDMInput,
            SurfaceIndex curSurfIndex, matrix_ref<uchar, N, 32> IDMOutput) {
  matrix<uint, 5, 8> src = 0;
  // mov(96)    VX(0, 0)<1>, UNIInput
  src.format<uint>().select<32, 1>(0) =
      UNIInput.format<uint>().select<32, 1>(0);
  // mov  (128)   VX(3,0)<1>,  IDMInput
  src.format<uint>().select<8, 1>(32) = IDMInput.format<uint>().select<8, 1>(0);

  unsigned fCtrl = 0;
  unsigned rspLength = N; // used to 16 in the previous platforms
  unsigned msglength = 5;
  uint Descriptor = (fCtrl & 0x7FFFF) + (1 << 19) + ((rspLength & 0x1f) << 20) +
                    ((msglength & 0xF) << 25);

  Descriptor = cm_get_value(curSurfIndex) + Descriptor;
  cm_send(IDMOutput.format<ushort, N, 16>(), src.format<ushort, 5, 16>(),
          __SFID_VME, Descriptor, 0u);
}

// Gen10 HEVC VME support

#define selrows(r) select<r, 1, 32, 1>(0, 0).format<ushort, r, 16>()

template <typename Dummy = void>
CM_INLINE void cm_vme_hevc_ime(/*const*/ matrix<uchar, 5, 32> IMEInput,
                               /*const*/ matrix<uchar, 10, 32> StreamInput,
                               int LengthStreamInput, SurfaceIndex curSurfIndex,
                               matrix_ref<uchar, 12, 32> StreamOutput) {
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

    cm_sends(StreamOutput.selrows(12), IMEInput.selrows(5),
             StreamInput.selrows(10), ExDescriptor, Descriptor, 0);
  } else
    cm_send(StreamOutput.selrows(12), IMEInput.selrows(5), __SFID_VME,
            Descriptor, 0);
}

template <typename Dummy = void>
CM_INLINE void cm_vme_hevc_sic(/*const*/ matrix<uchar, 5, 32> SICInput,
                               /*const*/ matrix<uchar, 8, 32> NPInput,
                               int LengthNP, SurfaceIndex curSurfIndex,
                               matrix_ref<uchar, 22, 32> SICOutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, SICInput.n_rows());
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor = SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_SIC_MSG);

  if (LengthNP > 0) {
    Descriptor =
        SEND_SET_Descriptor_Num_Outputs(Descriptor, SICOutput.n_rows());
    U32 ExDescriptor = SEND_SET_Descriptor_Num_InputsExt(__SFID_CRE, LengthNP);

    cm_sends(SICOutput.selrows(22), SICInput.selrows(5), NPInput.selrows(8),
             ExDescriptor, Descriptor, 0);
  } else {
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, 2);

    cm_send(SICOutput.selrows(2), SICInput.selrows(5), __SFID_CRE, Descriptor,
            0);
  }
}

template <typename Dummy = void>
CM_INLINE void cm_vme_hevc_sc(/*const*/ matrix<uchar, 5, 32> SICInput,
                              SurfaceIndex curSurfIndex,
                              matrix_ref<uchar, 2, 32> SICOutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, SICInput.n_rows());
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor = SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_SIC_MSG);
  Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, SICOutput.n_rows());

  cm_send(SICOutput.selrows(2), SICInput.selrows(5), __SFID_CRE, Descriptor, 0);
}

template <typename Dummy = void>
CM_INLINE void
cm_vme_hevc_hpm_u(/*const*/ matrix<uchar, 12, 32> HPMInput,
                  /*const*/ matrix<uchar, 10, 32> StreamInputInter,
                  SurfaceIndex curSurfIndex,
                  matrix_ref<uchar, 23, 32> HPMOutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);
  U32 ExDescriptor =
      SEND_SET_Descriptor_Num_InputsExt(__SFID_CRE, StreamInputInter.n_rows());

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, HPMInput.n_rows());
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor = SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_HPM_MSG);
  Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, HPMOutput.n_rows());

  cm_sends(HPMOutput.selrows(23), HPMInput.selrows(12),
           StreamInputInter.selrows(10), ExDescriptor, Descriptor, 0);
}

template <typename Dummy = void>
CM_INLINE void
cm_vme_hevc_hpm_b(/*const*/ matrix<uchar, 12, 32> HPMInput,
                  /*const*/ matrix<uchar, 20, 32> StreamInputInter,
                  int LengthStreamInputInter, SurfaceIndex curSurfIndex,
                  matrix_ref<uchar, 23, 32> HPMOutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);
  U32 ExDescriptor =
      SEND_SET_Descriptor_Num_InputsExt(__SFID_CRE, LengthStreamInputInter);

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, HPMInput.n_rows());
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor = SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_HPM_MSG);
  Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, HPMOutput.n_rows());

  cm_sends(HPMOutput.selrows(23), HPMInput.selrows(12),
           StreamInputInter.selrows(20), ExDescriptor, Descriptor, 0);
}

template <typename Dummy = void>
CM_INLINE void cm_vme_hevc_fbr(/*const*/ matrix<uchar, 3, 32> UNIInput,
                               /*const*/ matrix<uchar, 16, 32> CUInput,
                               int ValidCULength, SurfaceIndex curSurfIndex,
                               matrix_ref<uchar, 18, 32> FBROutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);
  U32 ExDescriptor =
      SEND_SET_Descriptor_Num_InputsExt(__SFID_CRE, ValidCULength);

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, UNIInput.n_rows());
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor = SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_FBR_MSG);
  Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, 2 + ValidCULength);

  cm_sends(FBROutput.selrows(18), UNIInput.selrows(3), CUInput.selrows(16),
           ExDescriptor, Descriptor, 0);
}

template <typename Dummy = void>
CM_INLINE void cm_vme_hevc_rpm(/*const*/ matrix<uchar, 7, 32> UNIInput,
                               /*const*/ matrix<uchar, 16, 32> CUInput,
                               int ValidCULength, SurfaceIndex curSurfIndex,
                               matrix_ref<uchar, 8, 32> RPMOutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);
  U32 ExDescriptor =
      SEND_SET_Descriptor_Num_InputsExt(__SFID_CRE, ValidCULength);

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, UNIInput.n_rows());
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor = SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_RPM_MSG);
  Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, RPMOutput.n_rows());

  cm_sends(RPMOutput.selrows(8), UNIInput.selrows(7), CUInput.selrows(16),
           ExDescriptor, Descriptor, 0);
}

template <typename Dummy = void>
CM_INLINE void cm_vme_hevc_srm(/*const*/ matrix<uchar, 8, 32> UNIInput,
                               /*const*/ matrix<uchar, 16, 32> CUInput,
                               int ValidCULength, SurfaceIndex curSurfIndex,
                               matrix_ref<uchar, 18, 32> SRMOutput) {
  U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);
  U32 ExDescriptor =
      SEND_SET_Descriptor_Num_InputsExt(__SFID_CRE, ValidCULength);

  Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, UNIInput.n_rows());
  Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
  Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
  Descriptor = SEND_SET_Descriptor_Function(Descriptor, __VME_HEVC_SRM_MSG);
  Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, 2 + ValidCULength);

  cm_sends(SRMOutput.selrows(18), UNIInput.selrows(8), CUInput.selrows(16),
           ExDescriptor, Descriptor, 0);
}

#endif /* _CLANG_gen10_VME_H_ */
